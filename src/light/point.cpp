#include"point.h"
#include"sampling.h"

namespace valley
{

// PointLight Method Definitions
Color PointLight::sample_Li(const Isect&ref, const Point2f &u,
	Vector3f *wi, Float *pdf,
	Visibility *vis) const 
{
	*wi = Normalize(pLight - ref.p);
	*pdf = 1.f;
	*vis = Visibility(ref, Isect(pLight));
	return I / DistanceSquared(pLight, ref.p);
}

Color PointLight::power() const { return I * 4 * Pi; }

Float PointLight::pdf_Li(const Isect&, const Vector3f &) const { return 0; }

Color PointLight::sample_Le(const Point2f &u1, const Point2f &u2, Ray *ray, 
	Normal3f *nLight, Float *pdfPos, Float *pdfDir) const
{
	*ray = Ray(pLight, uniform_sample_Sphere(u1), Infinity /*mediumInterface.inside*/);
	*nLight = (Normal3f)ray->d;
	*pdfPos = 1;
	*pdfDir = uniform_sphere_pdf();
	return I;
}

void PointLight::pdf_Le(const Ray &, const Normal3f &, Float *pdfPos,
	Float *pdfDir) const 
{
	*pdfPos = 0;
	*pdfDir = uniform_sphere_pdf();
}

}	//namespace valley
