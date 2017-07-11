#include"diffuse.h"
#include"shape.h"
#include"rng.h"
#include"sampling.h"

namespace valley
{

// DiffuseAreaLight Method Definitions
DiffuseAreaLight::DiffuseAreaLight(const Transform &LightToWorld,
	//const MediumInterface &mediumInterface,
	const Color& Lemit, int nSamples,
	const std::shared_ptr<Shape> &shape,
	bool twoSided)
	: AreaLight(LightToWorld,  /*mediumInterface,*/ nSamples),
	Lemit(Lemit),
	shape(shape),
	twoSided(twoSided),
	area(shape->area()) {
	// Warn if light has transformation with non-uniform scale, though not
	// for Triangles, since this doesn't matter for them.
	//if (WorldToLight.HasScale() &&
	//	dynamic_cast<const Triangle *>(shape.get()) == nullptr)
	//	std::cerr << "diffuseAreaLight error";
}

Color DiffuseAreaLight::power() const 
{
	return Lemit * (twoSided ? 2 : 1) * area * Pi;
}

Color DiffuseAreaLight::sample_Li(const Isect& ref, const Point2f &u,
	Vector3f *wi, Float *pdf,
	Visibility *vis) const 
{
	Isect pShape = shape->sample(ref, u, pdf);
	//pShape.mediumInterface = mediumInterface;
	if (*pdf == 0 || (pShape.p - ref.p).LengthSquared() == 0) {
		*pdf = 0;
		return 0.f;
	}
	*wi = Normalize(pShape.p - ref.p);
	*vis = Visibility(ref, pShape);
	return L(pShape, -*wi);
}

Float DiffuseAreaLight::pdf_Li(const Isect&ref,
	const Vector3f &wi) const 
{
	return shape->pdf(ref, wi);
}

Color DiffuseAreaLight::sample_Le(const Point2f &u1, const Point2f &u2,
	Float time, Ray *ray, Normal3f *nLight,
	Float *pdfPos, Float *pdfDir) const 
{
	// Sample a point on the area light's _Shape_, _pShape_
	Isect pShape = shape->sample(u1, pdfPos);
	//pShape.mediumInterface = mediumInterface;
	*nLight = pShape.n;

	// Sample a cosine-weighted outgoing direction _w_ for area light
	Vector3f w;
	if (twoSided) {
		Point2f u = u2;
		// Choose a side to sample and then remap u[0] to [0,1] before
		// applying cosine-weighted hemisphere sampling for the chosen side.
		if (u[0] < .5) {
			u[0] = std::min(u[0] * 2, OneMinusEpsilon);
			w = cosine_sample_hemisphere(u);
		}
		else {
			u[0] = std::min((u[0] - .5f) * 2, OneMinusEpsilon);
			w = cosine_sample_hemisphere(u);
			w.z *= -1;
		}
		*pdfDir = 0.5f * cosine_hemisphere_pdf(std::abs(w.z));
	}
	else {
		w = cosine_sample_hemisphere(u2);
		*pdfDir = cosine_hemisphere_pdf(w.z);
	}

	Vector3f v1, v2, n(pShape.n);
	CoordinateSystem(n, &v1, &v2);
	w = w.x * v1 + w.y * v2 + w.z * n;
	*ray = pShape.generate_ray(w);
	return L(pShape, w);
}

void DiffuseAreaLight::pdf_Le(const Ray &ray, const Normal3f &n, Float *pdfPos,
	Float *pdfDir) const 
{
	Isect it(ray.o, n, Vector3f(), Vector3f(n));
	*pdfPos = shape->pdf(it);
	*pdfDir = twoSided ? (.5 *cosine_hemisphere_pdf(AbsDot(n, ray.d)))
		: cosine_hemisphere_pdf(Dot(n, ray.d));
}

}	//namespace valley
