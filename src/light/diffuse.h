#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_LIGHT_DIFFUSE_H
#define VALLEY_LIGHT_DIFFUSE_H

#include"valley.h"
#include"light.h"

namespace valley
{

class DiffuseAreaLight : public AreaLight
{
public:
	// DiffuseAreaLight Public Methods
	DiffuseAreaLight(const Transform &LightToWorld,
		//const MediumInterface &mediumInterface, 
		const Color& Le, int nSamples, const std::shared_ptr<Shape> &shape,
		bool twoSided = false);

	Color L(const Isect&intr, const Vector3f &w) const {
		return (twoSided || Dot(intr.n, w) > 0) ? Lemit : Color(0.f);
	}

	Color power() const;

	Float pdf_Li(const Isect&, const Vector3f &) const;
	void  pdf_Le(const Ray &, const Normal3f &, Float *pdfPos,
				Float *pdfDir) const;

	//区域光源的sample_Li方法与面积有关
	Color sample_Li(const Isect&ref, const Point2f &u, Vector3f *wo,
					Float *pdf, Visibility* vis) const;

	Color sample_Le(const Point2f &u1, const Point2f &u2,  Ray *ray, 
					Normal3f *nLight, Float *pdfPos, Float *pdfDir) const override;

protected:
	// DiffuseAreaLight Protected Data
	const Color Lemit;
	std::shared_ptr<Shape> shape;
	// Added after book publication: by default, DiffuseAreaLights still
	// only emit in the hemimsphere around the surface normal.  However,
	// this behavior can now be overridden to give emission on both sides.
	const bool twoSided;
	const Float area;
};

}	//namespace valley


#endif //VALLEY_LIGHT_DIFFUSE_H
