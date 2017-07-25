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

int generate_camera_subpath(const Scene &scene, Sampler &sampler, int maxDepth, 
	Vertex *path, const Camera &camera, const Point2f &pFilm);

int generate_light_subpath(const Scene &scene, Sampler &sampler, int maxDepth,
	Vertex *path, const Distribution1D &lightDistr,
	const std::unordered_map<const Light *, size_t> &lightToIndex);

Color connect_vertex(
	const Scene &scene, Vertex *lightVertices, Vertex *cameraVertices, int s,
	int t, const Distribution1D &lightDistr,
	const std::unordered_map<const Light *, size_t> &lightToIndex,
	const Camera &camera, Sampler &sampler, Point2f *pRaster,
	Float *misWeight = nullptr);

struct Vertex
{
	VertexType type;
	Color beta;
	bool delta = false;
	Float pdfFwd = 0, pdfRev = 0;
	union {
		EndpointIsect ei;
		//MediumIsect mi;
		SurfaceIsect si;
	};

	Vertex() : ei() {}
	Vertex(VertexType type, const EndpointIsect &ei, const Color &beta)
		: type(type), beta(beta), ei(ei) {}
	Vertex(const SurfaceIsect &si, const Color &beta)
		: type(VertexType::Surface), beta(beta), si(si) {}

	static Vertex create_light(const Light *light, const Ray &ray,
		const Normal3f &nLight, const Color& Le, Float pdf)
	{
		Vertex v(VertexType::Light, new EndpointIsect(light, ray, nLight), Le);
		v.pdfFwd = pdf;
		return v;
	}

	bool is_InfiniteLight() const
	{
		return type == VertexType::Light &&
			(!ei.light || ei.light->flags & (int)LightFlags::Infinite ||
			ei.light->flags & (int)LightFlags::DeltaDirection);
	}

};


class BDPT : public Integrator
{
public:
	BDPT(std::shared_ptr<Camera> camera, std::shared_ptr<Sampler> sampler,
		int maxDepth) : Integrator(camera, sampler, maxDepth) {}
	~BDPT() {}

	void render(const Scene& scene);

private:

	const std::string lightSampleStrategy;

	std::unique_ptr<LightDistribution> lightDistribution;
};

}	//namespace valley


#endif //VALLEY_INTEGRATOR_BDPT_H
