#include"primitive.h"
#include"intersection.h"
#include"shape.h"

namespace valley
{

// GeometricPrimitive Method Definitions
Bounds3f GeometricPrimitive::world_bound() const { return shape->WorldBound(); }

bool GeometricPrimitive::intersectP(const Ray &r) const { return shape->IntersectP(r); }

bool GeometricPrimitive::intersect(
	const Ray& r, SurfaceIsect* isect) const 
{
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

const AreaLight *GeometricPrimitive::get_AreaLight() const {
	return areaLight.get();
}

const Material *GeometricPrimitive::get_material() const {
	return material.get();
}

void GeometricPrimitive::compute_scattering(
	SurfaceIsect *isect, 
//	MemoryArena &arena,
	TransportMode mode,
	bool allowMultipleLobes) const {

	if (material)
		material->compute_scattering(isect, mode, allowMultipleLobes);
	CHECK_GE(Dot(isect->n, isect->shading.n), 0.);
}


// Aggregate Method Definitions
const AreaLight* Aggregate::get_AreaLight() const
{
	LOG(FATAL) <<
		"Aggregate::GetAreaLight() method"
		"called; should have gone to GeometricPrimitive";
	return nullptr;
}

const Material* Aggregate::get_material() const
{
	LOG(FATAL) <<
		"Aggregate::GetMaterial() method"
		"called; should have gone to GeometricPrimitive";
	return nullptr;
}

void Aggregate::compute_scattering(
	SurfaceIsect* isect,
//	MemoryArena &arena,
	TransportMode mode,
	bool allowMultipleLobes) const 
{
	LOG(FATAL) <<
		"Aggregate::ComputeScatteringFunctions() method"
		"called; should have gone to GeometricPrimitive";
}

}	//namespace valley