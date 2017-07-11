#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_LIGHT_DISTANCE_H
#define VALLEY_LIGHT_DISTANCE_H

#include"valley.h"
#include"light.h"


namespace valley
{

// DistantLight Declarations
class DistantLight : public Light 
{
public:
	DistantLight(const Transform &LightToWorld, const Color &L,
				 const Vector3f &w);

	Color power() const;

	//有向光源的一些方法需要知道场景包围盒的大小
	void preprocess(const Scene &scene);

	Float pdf_Li(const Isect&, const Vector3f&) const;
	void pdf_Le(const Ray&, const Normal3f&, Float* pdfPos,
				Float* pdfDir) const;

	Color sample_Li(const Isect& ref, const Point2f& u, Vector3f* wi,
					Float* pdf, Visibility* vis) const override;
	Color sample_Le(const Point2f& u1, const Point2f& u2, Ray* ray, 
					Normal3f* nLight, Float* pdfPos, Float* pdfDir) const override;

private:
	const Color L;
	const Vector3f wLight;
	Point3f worldCenter;
	Float worldRadius;
};

}	//namespace valley


#endif //VALLEY_LIGHT_DISTANCE_H
