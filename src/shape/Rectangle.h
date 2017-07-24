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
		bool reverseOrientation, Float Xaxis, Float Yaxis) : 
		Shape(o2w, new Transform(o2w->GetInverseMatrix(), o2w->GetMatrix()), 
		reverseOrientation), point(-Xaxis / 2.f, -Yaxis / 2.f, 0), 
		first(Xaxis, 0, 0), second(0, Yaxis, 0),
		normal(Normalize(Cross(first, second))) {}	//我们现在在左手系中

	Bounds3f object_bound() const;

	bool intersect(const Ray &ray, SurfaceIsect* isect
		/*bool testAlphaTexture*/) const;
	bool intersectP(const Ray &ray, bool testAlphaTexture) const;

	Float area() const;

	Isect sample(const Point2f &u, Float *pdf) const;

private:
	Point3f 		point; //可以通过变换矩阵来定义位置，但point不是多余的
	Vector3f		first, second;
	Normal3f        normal;
};

}	//namespace valley


#endif //VALLEY_CORE_RECTANGLE_H
