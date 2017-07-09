#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_SPHERE_H
#define VALLEY_CORE_SPHERE_H

#include"valley.h"
#include"geometry.h"
#include"intersection.h"

namespace valley
{

class Sphere
{
protected:
	Point3f center;
	Float radius;

	const Float kEpsilon = 0.001f;   //for shadows and secondary rays

public:
	Sphere(Point3f c, Float r) : center(c), radius(r) {}
	
	bool intersect(const Ray& ray, Isect& isect) const
	{
		Float t;
		Vector3f v = ray.o - center;
		//求解t^2 + 2bt + c = 0;		因为direction是单位向量，所以a = d * d = 1
		Float b = ray.d * v;
		Float c = v * v - radius * radius;
		Float discr = b * b - c;		

		if (discr > 0.0)
		{
			Float e = sqrt(discr);

			t = -b - e;
			if (t > kEpsilon)
			{
				//tmin = t;										//光线原点在球体内部时应翻转法线
				isect.n = (v + t * ray.d) / radius;			//返回的并不是单位向量
				isect.p = ray.o + t * ray.d;
				return true;
			}

			t = -b + e;
			if (t > kEpsilon)
			{
				//tmin = t;
				isect.n = (v + t * ray.d) / radius;
				isect.p = ray.o + t * ray.d;
				return true;
			}
			return false;	//并非所有路径都返回的意思
		}
		else
			return false;
	}
};

}	//namespace valley


#endif //VALLEY_CORE_SPHERE_H
