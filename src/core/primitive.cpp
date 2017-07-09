#include"primitive.h"
#include"intersection.h"
#include"shape.h"

namespace valley
{

// GeometricPrimitive Method Definitions
Bounds3f GeometricPrimitive::WorldBound() const { return shape->WorldBound(); }

bool GeometricPrimitive::IntersectP(const Ray &r) const {
	return shape->IntersectP(r);
}

bool GeometricPrimitive::Intersect(const Ray &r,
	SurfaceIsect* isect) const {
	Float tHit;
	if (!shape->Intersect(r, &tHit, isect)) return false;
	r.tMax = tHit;
	isect->primitive = this;
	CHECK_GE(Dot(isect->n, isect->shading.n), 0.);
	// Initialize _SurfaceInteraction::mediumInterface_ after _Shape_
	// intersection
	//if (mediumInterface.IsMediumTransition())
	//	isect->mediumInterface = mediumInterface;
	//else
	//	isect->mediumInterface = MediumInterface(r.medium);
	return true;
}

const AreaLight *GeometricPrimitive::GetAreaLight() const {
	return areaLight.get();
}

const Material *GeometricPrimitive::GetMaterial() const {
	return material.get();
}

void GeometricPrimitive::compute_scattering(
	SurfaceIsect *isect, TransportMode mode,
	bool allowMultipleLobes) const {

	if (material)
		material->compute_scattering(isect, mode, allowMultipleLobes);
	CHECK_GE(Dot(isect->n, isect->shading.n), 0.);
}

}	//namespace valley