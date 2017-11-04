#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_LIGHT_DIFFUSE_H
#define RAINY_LIGHT_DIFFUSE_H

#include"rainy.h"
#include"light.h"

namespace rainy
{

class DiffuseAreaLight : public AreaLight
{
public:
	// DiffuseAreaLight Public Methods
	DiffuseAreaLight(const Transform &LightToWorld,
		//const MediumInterface &mediumInterface, 
		const Spectrum& Le, int nSamples, const std::shared_ptr<Shape> &shape,
		bool twoSided = false);

	Spectrum L(const Interaction&intr, const Vector3f &w) const 
	{
		return (twoSided || Dot(intr.n, w) > 0) ? Lemit : Spectrum(0.f);
	}

	Spectrum power() const;

	Float pdf_Li(const Interaction&, const Vector3f&) const;
	void  pdf_Le(const Ray&, const Normal3f&, Float* pdfPos,
				 Float *pdfDir) const;

	//区域光源的sample_Li方法与面积有关
	Spectrum sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wo,
					Float* pdf, Visibility* vis) const;

	Spectrum sample_Le(const Point2f& u1, const Point2f& u2,  Ray* ray, 
					Normal3f* nLight, Float* pdfPos, Float* pdfDir) const override;

protected:
	// DiffuseAreaLight Protected Data
	const Spectrum Lemit;
	std::shared_ptr<Shape> shape;
	// Added after book publication: by default, DiffuseAreaLights still
	// only emit in the hemimsphere around the surface normal.  However,
	// this behavior can now be overridden to give emission on both sides.
	const bool twoSided;
	const Float area;
};

}	//namespace rainy


#endif //RAINY_LIGHT_DIFFUSE_H
