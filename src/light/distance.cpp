#include"distance.h"
#include"scene.h"
#include"sampling.h"

namespace valley
{

// DistantLight Method Definitions
DistantLight::DistantLight(const Transform &LightToWorld, const Color& L,
	const Vector3f &wLight)
	: Light((int)Light_type::DeltaDirection, LightToWorld /*MediumInterface()*/),
	L(L),
	wLight(Normalize(LightToWorld(wLight))) {}

Color DistantLight::power() const
{
	return L * Pi * worldRadius * worldRadius;	//计算结果大于实际，但满足算法要求
}

void DistantLight::preprocess(const Scene& scene)
{
	scene.WorldBound().BoundingSphere(&worldCenter, &worldRadius);
}

Float DistantLight::pdf_Li(const Isect&, const Vector3f&) const
{
	return 0.f;
}

void DistantLight::pdf_Le(const Ray&, const Normal3f&, Float* pdfPos,
	Float* pdfDir) const
{
	*pdfPos = 1 / (Pi * worldRadius * worldRadius);
	*pdfDir = 0;
}

Color DistantLight::sample_Li(const Isect& ref, const Point2f& u, Vector3f* wi,
	Float* pdf, Visibility* vis) const
{
	*wi = wLight;
	*pdf = 1;
	Point3f pOutside = ref.p + wLight * (2 * worldRadius);	//大于两倍的世界半径
	*vis =
		Visibility(ref, Isect(pOutside));
	return L;
}

Color DistantLight::sample_Le(const Point2f& u1, const Point2f& u2, Ray* ray,
	Normal3f* nLight, Float* pdfPos, Float* pdfDir) const
{
	// Choose point on disk oriented toward infinite light direction
	Vector3f v1, v2;
	CoordinateSystem(wLight, &v1, &v2);
	Point2f cd = concentric_sample_disk(u1);
	Point3f pDisk = worldCenter + worldRadius * (cd.x * v1 + cd.y * v2);

	// Set ray origin and direction for infinite light ray
	*ray = Ray(pDisk + worldRadius * wLight, -wLight, Infinity);
	*nLight = (Normal3f)ray->d;
	*pdfPos = 1 / (Pi * worldRadius * worldRadius);
	*pdfDir = 1;
	return L;
}

}	//namespace valley
