#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_INTEGRATOR_BDPT_H
#define VALLEY_INTEGRATOR_BDPT_H

#include"valley.h"
#include"integrator.h"
#include"light.h"

namespace valley
{

enum class VertexType { Camera, Light, Surface, Medium };
struct Vertex;

Float InfiniteLight_density(
	const Scene &scene, const Distribution1D &lightDistr, const Vector3f &w,
	const std::unordered_map<const Light *, size_t> &lightToDistrIndex);

Float correct_ShadingNormal(const SurfaceInteraction &isect, const Vector3f &wo,
	const Vector3f &wi, TransportMode mode);

int generate_camera_subpath(const Scene &scene, Sampler &sampler, int maxDepth, 
	Vertex *path, const Camera &camera, const Point2f &pFilm);

int generate_light_subpath(const Scene &scene, Sampler &sampler, int maxDepth,
	Vertex *path, const Distribution1D &lightDistr,
	const std::unordered_map<const Light *, size_t> &lightToIndex);

Spectrum connect_vertex(
	const Scene &scene, Vertex *lightVertices, Vertex *cameraVertices, int s, int t, 
	const Distribution1D &lightDistr,
	const std::unordered_map<const Light *, size_t> &lightToIndex,
	const Camera &camera, Sampler &sampler, Point2f *pRaster,
	Float *misWeight = nullptr);

struct Vertex
{
	VertexType type;
	Spectrum beta;
	bool delta = false;
	Float pdfFwd = 0, pdfRev = 0;
	union {
		EndpointInteraction ei;
		//MediumInteraction mi;
		SurfaceInteraction si;
	};

	Vertex() : ei() {}
	Vertex(VertexType type, const EndpointInteraction& ei, const Spectrum &beta)
		: type(type), beta(beta), ei(ei) {}
	Vertex(const SurfaceInteraction &si, const Spectrum &beta)
		: type(VertexType::Surface), beta(beta), si(si) {}
	Vertex(const Vertex &v) { memcpy(this, &v, sizeof(Vertex)); }
	Vertex &operator=(const Vertex &v) {
		if(this != &v) memcpy(this, &v, sizeof(Vertex));
		return *this; }
	~Vertex(){}


	const Interaction &get_interaction() const {
		switch (type) {
			//case VertexType::Medium:
			//	return mi;
		case VertexType::Surface:
			return si;
		default:
			return ei;
		}
	}
	const Point3f &p()   const { return get_interaction().p; }
	const Normal3f &ng() const { return get_interaction().n; }
	bool  on_surface()   const { return ng() != Normal3f(); }
	const Normal3f &ns() const {
		if (type == VertexType::Surface)
			return si.shading.n;
		else
			return get_interaction().n;
	}


	static Vertex create_camera(const Camera *camera, const Ray &ray,
		const Spectrum &beta) 
	{
		return Vertex(VertexType::Camera, EndpointInteraction(camera, ray), beta);
	}

	static Vertex create_camera(const Camera *camera, const Interaction &it,
		const Spectrum &beta) 
	{
		return Vertex(VertexType::Camera, EndpointInteraction(it, camera), beta);
	}

	static Vertex create_light(const Light *light, const Ray &ray,
		const Normal3f &nLight, const Spectrum& Le, Float pdf)
	{
		Vertex v(VertexType::Light, EndpointInteraction(light, ray, nLight), Le);
		v.pdfFwd = pdf;
		return v;
	}

	static Vertex create_light(const EndpointInteraction &ei,
		const Spectrum &beta, Float pdf) 
	{
		Vertex v(VertexType::Light, ei, beta);
		v.pdfFwd = pdf;
		return v;
	}

	static Vertex create_surface(const SurfaceInteraction &si,
		const Spectrum &beta, Float pdf,
		const Vertex &prev) 
	{
		Vertex v(si, beta);
		v.pdfFwd = prev.convert_density(pdf, v);
		return v;
	}


	bool is_connectible() const 
	{
		switch (type) 
		{
		case VertexType::Medium:
			return true;
		case VertexType::Light:
			return (ei.light->flags & (int)LightType::DeltaDirection) == 0;
		case VertexType::Camera:
			return true;
		case VertexType::Surface:
			return si.bsdf->components_num(BxDFType::NonSpecular) > 0;
		}
		LOG(FATAL) << "Unhandled vertex type in IsConnectable()";
		return false;  // NOTREACHED
	}
	bool is_Light() const {
		return type == VertexType::Light ||
			(type == VertexType::Surface && si.primitive->get_AreaLight());
	}
	bool is_DeltaLight() const {
		return type == VertexType::Light && ei.light &&
			valley::is_DeltaLight(ei.light->flags);
	}
	bool is_InfiniteLight() const
	{
		return type == VertexType::Light &&
			(!ei.light || ei.light->flags & (int)LightType::Infinite ||
			ei.light->flags & (int)LightType::DeltaDirection);
	}

	Spectrum f(const Vertex &next, TransportMode mode) const
	{
		Vector3f wi = next.p() - p();
		if (wi.LengthSquared() == 0) return 0.;
		wi = Normalize(wi);
		switch (type) 
		{
		case VertexType::Surface:
			return si.bsdf->f(si.wo, wi) *
				correct_ShadingNormal(si, si.wo, wi, mode);
		//case VertexType::Medium:
		//	return mi.phase->p(mi.wo, wi);
		default:
			LOG(FATAL) << "Vertex::f(): Unimplemented";
			return Spectrum(0.f);
		}
	}

	Spectrum Le(const Scene &scene, const Vertex &v) const
	{
		if (!is_Light()) return Spectrum(0.f);
		Vector3f w = v.p() - p();
		if (w.LengthSquared() == 0) return 0.;
		w = Normalize(w);
		if (is_InfiniteLight())
		{
			// Return emitted radiance for infinite light sources
			Spectrum Le(0.f);
			for (const auto &light : scene.infiniteLights)
				Le += light->Le(Ray(p(), -w));
			return Le;
		}
		else 
		{
			const AreaLight *light = si.primitive->get_AreaLight();
			CHECK_NOTNULL(light);
			return light->L(si, w);
		}
	}

	//将基于角度的 pdf 转换为基于面积形式
	Float convert_density(Float pdf, const Vertex &next) const 
	{
		// Return solid angle density if _next_ is an infinite area light
		if (next.is_InfiniteLight()) return pdf;
		Vector3f w = next.p() - p();
		if (w.LengthSquared() == 0) return 0;
		Float invDist2 = 1 / w.LengthSquared();
		if (next.on_surface())
			pdf *= AbsDot(next.ng(), w * std::sqrt(invDist2));
		return pdf * invDist2;
	}

	Float pdf(const Scene &scene, const Vertex *prev,
		const Vertex &next) const 
	{
		if (type == VertexType::Light) return pdf_Light(scene, next);
		// Compute directions to preceding and next vertex
		Vector3f wn = next.p() - p();
		if (wn.LengthSquared() == 0) return 0;
		wn = Normalize(wn);
		Vector3f wp;
		if (prev)
		{
			wp = prev->p() - p();
			if (wp.LengthSquared() == 0) return 0;
			wp = Normalize(wp);
		}
		else
			CHECK(type == VertexType::Camera);

		// Compute directional density depending on the vertex types
		Float pdf = 0, unused;
		if (type == VertexType::Camera)
			ei.camera->pdf_We(ei.generate_ray(wn), &unused, &pdf);
		else if (type == VertexType::Surface)
			pdf = si.bsdf->pdf(wp, wn);
		//else if (type == VertexType::Medium)
		//	pdf = mi.phase->p(wp, wn);
		else
			LOG(FATAL) << "Vertex::Pdf(): Unimplemented";

		// Return probability per unit area at vertex _next_
		return convert_density(pdf, next);
	}

	Float pdf_Light(const Scene &scene, const Vertex &v) const
	{
		Vector3f w = v.p() - p();
		Float invDist2 = 1 / w.LengthSquared();
		w *= std::sqrt(invDist2);	//归一化
		Float pdf;
		if (is_InfiniteLight()) 
		{
			// Compute planar sampling density for infinite light sources
			Point3f worldCenter;
			Float worldRadius;
			scene.WorldBound().BoundingSphere(&worldCenter, &worldRadius);
			pdf = 1 / (Pi * worldRadius * worldRadius);
		}
		else 
		{
			// Get pointer _light_ to the light source at the vertex
			CHECK(is_Light());
			const Light *light = type == VertexType::Light
				? ei.light
				: si.primitive->get_AreaLight();
			CHECK_NOTNULL(light);

			// Compute sampling density for non-infinite light sources
			Float pdfPos, pdfDir;
			light->pdf_Le(Ray(p(), w), ng(), &pdfPos, &pdfDir);
			pdf = pdfDir * invDist2;
		}
		if (v.on_surface()) pdf *= AbsDot(v.ng(), w);
		return pdf;
	}

	Float pdf_LightOrigin(const Scene &scene, const Vertex &v,
		const Distribution1D &lightDistr,
		const std::unordered_map<const Light *, size_t>
		&lightToDistrIndex) const 
	{
		Vector3f w = v.p() - p();
		if (w.LengthSquared() == 0) return 0.;
		w = Normalize(w);
		if (is_InfiniteLight()) 
		{
			// Return solid angle density for infinite light sources
			return InfiniteLight_density(scene, lightDistr, w, lightToDistrIndex);
		}
		else 
		{
			// Return solid angle density for non-infinite light sources
			Float pdfPos, pdfDir, pdfChoice = 0;

			// Get pointer _light_ to the light source at the vertex
			CHECK(is_Light());
			const Light *light = type == VertexType::Light
				? ei.light
				: si.primitive->get_AreaLight();
			CHECK_NOTNULL(light);

			// Compute the discrete probability of sampling _light_, _pdfChoice_
			CHECK(lightToDistrIndex.find(light) != lightToDistrIndex.end());
			size_t index = lightToDistrIndex.find(light)->second;
			pdfChoice = lightDistr.discrete_pdf(index);

			light->pdf_Le(Ray(p(), w), ng(), &pdfPos, &pdfDir);
			return pdfPos * pdfChoice;
		}
	}
};


class BDPT : public Integrator
{
public:
	BDPT(const Scene &scene, std::shared_ptr<Camera> camera, std::shared_ptr<Sampler> sampler,
		int cameraDepth = 3, int lightDepth = 3, int pahtDepth = 5) : 
		Integrator(camera, sampler), cameraDepth(cameraDepth), 
		lightDepth(lightDepth), pathDepth(pahtDepth)
	{
		lightDistribution = std::unique_ptr<LightDistribution>{ new PowerDistribution(scene) };

		for (size_t i = 0; i < scene.lights.size(); ++i)
			lightToIndex[scene.lights[i].get()] = i;
	}
	~BDPT() {}

	void render(const Scene& scene);
	void interactive(const Scene& scene, int x, int y);

private:
	int cameraDepth, lightDepth, pathDepth;
	const std::string lightSampleStrategy;
	std::unique_ptr<LightDistribution> lightDistribution;

	//从光源指针到 scene.lights/lightDistr 中的偏移量的映射
	std::unordered_map<const Light*, size_t> lightToIndex;
};

}	//namespace valley


#endif //VALLEY_INTEGRATOR_BDPT_H
