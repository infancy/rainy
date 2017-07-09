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

//Intersection 
class Isect
{
public:
	Isect(Float maxDist = Infinity) : dist(maxDist) {}
	Isect(const Point3f &p, const Normal3f &n, const Vector3f &pError,
		const Vector3f &wo, Float maxDist = Infinity);

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
		return Ray(origin, d, 1 - ShadowEpsilon);
	}

	Ray generate_ray(const Isect& isect) const 
	{
		Point3f origin = offset_ray_origin(p, pError, n, isect.p - p);
		Point3f target = offset_ray_origin(isect.p, isect.pError, isect.n, origin - isect.p);
		Vector3f d = target - origin;
		return Ray(origin, d, 1 - ShadowEpsilon);
	}

private:
	//对光线源点进行一定量的偏移，以抵消数值精度带来的误差
	Point3f offset_ray_origin(const Point3f& p, const Vector3f& pError,
							  const Normal3f& normal, const Vector3f& direct) const
	{
		Float d = Dot(Abs(normal), pError);
	#ifdef VALLEY_FLOAT_AS_DOUBLE
		// We have tons of precision; for now bump up the offset a bunch just
		// to be extra sure that we start on the right side of the surface
		// (In case of any bugs in the epsilons code...)
		d *= 1024.;
	#endif
		Vector3f offset = d * Vector3f(normal);
		if (Dot(direct, normal) < 0) offset = -offset;	//法线与生成光线的方向
		Point3f po = p + offset;
		// Round offset point _po_ away from _p_
		for (int i = 0; i < 3; ++i) {
			if (offset[i] > 0)
				po[i] = NextFloatUp(po[i]);
			else if (offset[i] < 0)
				po[i] = NextFloatDown(po[i]);
		}
		return po;
	}

public:
	Float dist;			//到最近交点的距离
	Point3f p;			//交点
	Normal3f n;    //交点处的法线
	Vector3f wo;		//入射光线的方向
	Vector3f pError;    //累积的浮点数绝对误差  
};

class SurfaceIsect : public Isect 
{
public:
	SurfaceIsect() {}
	SurfaceIsect(const Point3f& p, const Vector3f& pError,
				 const Point2f& uv, const Vector3f& wo,
				 const Vector3f& dpdu, const Vector3f& dpdv,
				 const Normal3f& dndu, const Normal3f& dndv, 
			     const Shape* sh);
	//设置着色几何体
	void set_shading_geometry(const Vector3f& dpdu, const Vector3f& dpdv,
							  const Normal3f& dndu, const Normal3f& dndv, 
							  bool orientationIsAuthoritative);

	void compute_scattering(const RayDifferential& ray, 
							TransportMode mode = TransportMode::Radiance, 
							bool allowMultipleLobes = true);

	//void compute_differentials(const RayDifferential &r) const;

	Color Le(const Vector3f& w) const;

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

}	//namespace valley


#endif //VALLEY_CORE_INTERSECTION_H
