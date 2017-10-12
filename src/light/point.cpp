#include"point.h"
#include"sampling.h"

namespace valley
{

// PointLight Method Definitions
Spectrum PointLight::power() const { return intensity * 4 * Pi; }

Float PointLight::pdf_Li(const Interaction&, const Vector3f &) const { return 0; }

void PointLight::pdf_Le(const Ray&, const Normal3f&, Float* pdfPos,
	Float* pdfDir) const
{
	*pdfPos = 0;	// 采样到这个点的概率为 0
	*pdfDir = uniform_sphere_pdf();
}

Spectrum PointLight::sample_Li(const Interaction& ref, const Point2f &u,
	Vector3f *wi, Float *pdf, Visibility *vis) const 
{
	*wi = Normalize(pLight - ref.p);
	*pdf = 1.f;
	*vis = Visibility(ref, Interaction(pLight));
	return intensity / DistanceSquared(pLight, ref.p);
}

Spectrum PointLight::sample_Le(const Point2f& u1, const Point2f& u2, Ray* ray,
	Normal3f* nLight, Float* pdfPos, Float* pdfDir) const
{
	*ray = Ray(pLight, uniform_sample_sphere(u1), Infinity /*mediumInterface.inside*/);
	*nLight = (Normal3f)ray->d;
	*pdfPos = 1;
	*pdfDir = uniform_sphere_pdf();	//
	return intensity;
}

}	//namespace valley
