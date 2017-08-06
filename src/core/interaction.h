#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_INTERSECTION_H
#define VALLEY_CORE_INTERSECTION_H

#include"valley.h"
#include"geometry.h"
#include"material.h"
#include"bsdf.h"

namespace valley
{

class Interaction
{
public:
	Interaction() {}
	Interaction(const Point3f &p  /*const MediumInterface &mediumInterface*/)
		: p(p) /*mediumInterface(mediumInterface)*/ {}
	Interaction(const Point3f &p, const Vector3f &wo 
		/*const MediumInterface &mediumInterface*/)
		: p(p), wo(wo) /*mediumInterface(mediumInterface)*/ {}
	Interaction(const Point3f &p, const Normal3f &n, const Vector3f &pError,
		const Vector3f &wo, /*const MediumInterface &mediumInterface, */ Float maxDist = Infinity);

	Ray generate_ray(const Vector3f& d) const
	{
		Point3f o = offset_ray_origin(p, pError, n, d);
		return Ray(o, d, Infinity);
	}

	//给予空间一点，生成射向该点的ray
	Ray generate_ray(const Point3f &p2) const 
	{
		Point3f origin = offset_ray_origin(p, pError, n, p2 - p);
		Vector3f d = p2 - p;
		//return Ray(origin, d, 1 - ShadowEpsilon);
		return Ray(origin, d, (p - p2).Length() * (1 - ShadowEpsilon));
	}

	Ray generate_ray(const Interaction& isect) const 
	{
		Point3f origin = offset_ray_origin(p, pError, n, isect.p - p);
		Point3f target = offset_ray_origin(isect.p, isect.pError, isect.n, origin - isect.p);
		Vector3f d = target - origin;
		//return Ray(origin, d, 1 - ShadowEpsilon);
		return Ray(origin, d, (p - isect.p).Length() * (1 - ShadowEpsilon));
	}

	bool on_surface() const { return n != Normal3f(0, 0, 0); }
	bool on_medium() const { return !on_surface(); }

public:
	Point3f p;			//交点
	Normal3f n;    //交点处的法线
	Vector3f wo;		//入射光线的方向
	Vector3f pError;    //累积的浮点数绝对误差 
	//MediumInterface mediumInterface;
};

class SurfaceInteraction : public Interaction 
{
public:
	SurfaceInteraction() {}
	SurfaceInteraction(const Point3f& p, const Vector3f& pError,
				 const Point2f& uv, const Vector3f& wo,
				 const Vector3f& dpdu, const Vector3f& dpdv,
				 const Normal3f& dndu, const Normal3f& dndv, 
			     const Shape* sh);
	//设置着色几何体
	void set_shading_geometry(const Vector3f& dpdu, const Vector3f& dpdv,
							  const Normal3f& dndu, const Normal3f& dndv, 
							  bool orientationIsAuthoritative);

	void compute_scattering(const RayDifferential& ray, 
							//MemoryArena &arena,
							TransportMode mode = TransportMode::Radiance, 
							bool allowMultipleLobes = true);

	//void compute_differentials(const RayDifferential &r) const;

	Spectrum Le(const Vector3f& w) const;

public:
	Point2f uv;				//基于表面参数化的UV坐标
	Vector3f dpdu, dpdv;	//参数偏微分，位于切平面上，可生成法线
	Normal3f dndu, dndv;	//表面法线变化的的偏微分

	const Shape*	 shape     = nullptr;  
	const Primitive* primitive = nullptr;
	
	std::unique_ptr<BSDF>  bsdf	  = nullptr;
	//std::unique_ptr<BSSRDF> bssrdf = nullptr;

	//存储由凹凸纹理或三角形网格逐顶点法线插值得到的着色法线等值
	struct
	{
		Normal3f n;			//shadingNormal
		Vector3f dpdu, dpdv;
		Normal3f dndu, dndv;
	} shading;
};

/*
class MediumInteraction : public Interaction
{
public:
	// MediumInteraction Public Methods
	MediumInteraction() : phase(nullptr) {}
	MediumInteraction(const Point3f &p, const Vector3f &wo, Float time,
		const Medium *medium, const PhaseFunction *phase)
		: Interaction(p, wo), phase(phase) {}

	bool is_valid() const { return phase != nullptr; }

	// MediumInteraction Public Data
	const PhaseFunction *phase;
};
*/

//记录相机或光源上的交点
class EndpointInteraction : public Interaction 
{
public:
	EndpointInteraction() : Interaction(), light(nullptr) {}
	EndpointInteraction(const Interaction &it, const Camera *camera)
		: Interaction(it), camera(camera) {}
	EndpointInteraction(const Camera *camera, const Ray &ray)
		: Interaction(ray.o), camera(camera) {}
	EndpointInteraction(const Light *light, const Ray &r, const Normal3f &nl)
		: Interaction(r.o), light(light) { n = nl; }
	EndpointInteraction(const Interaction &it, const Light *light)
		: Interaction(it), light(light) {}
	EndpointInteraction(const Ray &ray)
		: Interaction(ray(1)), light(nullptr) { n = Normal3f(-ray.d); }

public:
	union
	{
		const Camera *camera;
		const Light *light;
	};
};

}	//namespace valley


#endif //VALLEY_CORE_INTERSECTION_H
