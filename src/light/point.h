#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_LIGHT_POINT_H
#define VALLEY_LIGHT_POINT_H

#include"valley.h"
#include"color.h"
#include"light.h"

namespace valley
{

class PointLight : public Light 
{
public:
	PointLight(const Transform& LightToWorld,
		//const MediumInterface &mediumInterface, 
		const Color& I) : 
		Light((int)Light_type::DeltaPosition, LightToWorld /*,mediumInterface*/),
		pLight(LightToWorld(Point3f(0, 0, 0))),
		I(I) {}

	Color power() const;

	Float pdf_Li(const Isect &, const Vector3f &) const;
	void pdf_Le(const Ray &, const Normal3f &, Float *pdfPos,
		Float *pdfDir) const;

	Color sample_Li(const Isect &ref, const Point2f &u, Vector3f *wi,
					Float *pdf, Visibility *vis) const;
	Color sample_Le(const Point2f &u1, const Point2f &u2, Ray *ray, 
					Normal3f *nLight, Float *pdfPos, Float *pdfDir) const;
	

private:
	const Point3f pLight;
	const Color I;
};

}	//namespace valley


#endif //VALLEY_LIGHT_POINT_H
