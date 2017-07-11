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
	Sphere(std::shared_ptr<Transform> ObjectToWorld,
		std::shared_ptr<Transform> WorldToObject,
		bool reverseOrientation, Float radius)
		: Shape(ObjectToWorld, WorldToObject, reverseOrientation),
		radius(radius) {}

	Bounds3f object_bound() const override;

	bool intersect(const Ray &ray, SurfaceIsect* isect
		/*bool testAlphaTexture*/) const override;
	bool intersectP(const Ray &ray, bool testAlphaTexture) const override;
	Float area() const override;

	Isect sample(const Point2f &u, Float *pdf) const override;
	Isect sample(const Isect& ref, const Point2f &u,
		Float *pdf) const override;

	Float pdf(const Isect& ref, const Vector3f &wi) const override;
	Float solid_angle(const Point3f &p, int nSamples) const override;

private:
	//Point3f center; 通过将光线转换到球的局部坐标中，使center总等于(0,0,0)
	Float radius;
};

}	//namespace valley


#endif //VALLEY_CORE_SPHERE_H
