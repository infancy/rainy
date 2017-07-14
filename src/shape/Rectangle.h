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
	Rectangle(Transform* o2w,
		bool reverseOrientation, Float Zaxis, Float Xaxis) : 
		Shape(o2w, new Transform(o2w->GetInverseMatrix(), o2w->GetMatrix()), 
		reverseOrientation), point(-Xaxis / 2.f, 0, -Zaxis / 2.f), 
		first(0, 0, Zaxis), second(Xaxis, 0, 0),
		normal(Normalize(Cross(first, second))) {}	//我们现在在左手系中

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
	Point3f 		point; //可以通过变换矩阵来定义位置，point是多余的
	Vector3f		first, second;
	Normal3f        normal;
};

}	//namespace valley


#endif //VALLEY_CORE_RECTANGLE_H
