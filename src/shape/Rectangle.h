#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_RECTANGLE_H
#define VALLEY_CORE_RECTANGLE_H

#include"shape.h"

namespace valley
{

class Rectangle : public Shape
{
public:
	Rectangle(std::shared_ptr<Transform> ObjectToWorld,
		std::shared_ptr<Transform> WorldToObject,
		bool reverseOrientation, Point3f p, Vector3f u, Vector3f r) : 
		Shape(ObjectToWorld, WorldToObject, reverseOrientation),
		point(p), up(u), right(r), normal(Normalize(Cross(right, up))) {}

	Bounds3f object_bound() const;

	bool intersect(const Ray &ray, SurfaceIsect* isect
		/*bool testAlphaTexture*/) const;
	bool intersectP(const Ray &ray, bool testAlphaTexture) const;

	Float area() const;

	Isect sample(const Point2f &u, Float *pdf) const;
	//Isect sample(const Isect& ref, const Point2f &u, Float *pdf) const;

	//Float pdf(const Isect& ref, const Vector3f &wi) const;
	//Float solid_angle(const Point3f &p, int nSamples) const;

private:
	Point3f 		point;   // corner vertex 
	Vector3f		up, right;
	Normal3f        normal;
};

}	//namespace valley


#endif //VALLEY_CORE_RECTANGLE_H
