#include"intersection.h"
#include"primitive.h"
#include"shape.h"
#include"color.h"
#include"light.h"

namespace valley
{ 

// Intersection Method Definitions
Isect::Isect(const Point3f &p, const Normal3f &n, const Vector3f &pError,
	const Vector3f &wo, Float maxDist) :
	p(p),
	pError(pError),
	wo(Normalize(wo)),
	n(n) {}

// SurfaceIntersection Method Definitions
SurfaceIsect::SurfaceIsect(
	const Point3f &p, const Vector3f &pError, const Point2f &uv,
	const Vector3f &wo, const Vector3f &dpdu, const Vector3f &dpdv,
	const Normal3f &dndu, const Normal3f &dndv, const Shape *shape)
	: Isect(p, Normal3f(Normalize(Cross(dpdu, dpdv))), pError, wo),
	uv(uv),
	dpdu(dpdu),
	dpdv(dpdv),
	dndu(dndu),
	dndv(dndv),
	shape(shape) {
	// Initialize shading geometry from true geometry
	shading.n = n;
	shading.dpdu = dpdu;
	shading.dpdv = dpdv;
	shading.dndu = dndu;
	shading.dndv = dndv;

	// Adjust normal based on orientation and handedness
	// 是否翻转法线方向 异或 矩阵的左右手系
	if (shape &&
		(shape->reverseOrientation ^ shape->transformSwapsHandedness)) 
	{
		n *= -1;
		shading.n *= -1;
	}
}

void SurfaceIsect::set_shading_geometry(const Vector3f &dpdus,
	const Vector3f &dpdvs,
	const Normal3f &dndus,
	const Normal3f &dndvs,
	bool orientationIsAuthoritative) {
	// Compute _shading.n_ for _SurfaceInteraction_
	shading.n = Normalize((Normal3f)Cross(dpdus, dpdvs));
	if (shape && (shape->reverseOrientation ^ shape->transformSwapsHandedness))
		shading.n = -shading.n;
	if (orientationIsAuthoritative)
		n = Faceforward(n, shading.n);
	else
		shading.n = Faceforward(shading.n, n);

	// Initialize _shading_ partial derivative values
	shading.dpdu = dpdus;
	shading.dpdv = dpdvs;
	shading.dndu = dndus;
	shading.dndv = dndvs;
}


void SurfaceIsect::compute_scattering(const RayDifferential &ray, TransportMode mode,
									  bool allowMultipleLobes)
{
	//compute_differentials(ray);

	primitive->compute_scattering(this, mode, allowMultipleLobes);
}

Color SurfaceIsect::Le(const Vector3f &w) const 
{
	const AreaLight* area = primitive->get_AreaLight();
	return area ? area->L(*this, w) : Color(0.f);
}

}