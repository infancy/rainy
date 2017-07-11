#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_SPHERE_H
#define VALLEY_CORE_SPHERE_H

#include"shape.h"

namespace valley
{

class Sphere : public Shape
{
public:
	Sphere(const Transform *ObjectToWorld, const Transform *WorldToObject,
		bool reverseOrientation, Float radius)
		: Shape(ObjectToWorld, WorldToObject, reverseOrientation),
		radius(radius) {}

	Bounds3f object_bound() const;

	bool intersect(const Ray &ray, Float *tHit, SurfaceIsect* isect,
				   bool testAlphaTexture) const;
	bool intersectP(const Ray &ray, bool testAlphaTexture) const;
	Float area() const;

	Isect sample(const Point2f &u, Float *pdf) const;
	Isect sample(const Isect& ref, const Point2f &u,
		Float *pdf) const;

	Float pdf(const Isect& ref, const Vector3f &wi) const;
	Float solid_angle(const Point3f &p, int nSamples) const;

private:
	//Point3f center; 通过将光线转换到球的局部坐标中，使center总等于(0,0,0)
	Float radius;
};

}	//namespace valley


#endif //VALLEY_CORE_SPHERE_H
