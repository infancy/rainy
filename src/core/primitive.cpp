#include"primitive.h"
#include"intersection.h"
#include"shape.h"

namespace valley
{

// GeometricPrimitive Method Definitions
Bounds3f GeometricPrimitive::world_bound() const { return shape->world_bound(); }

bool GeometricPrimitive::intersectP(const Ray &r) const { return shape->intersectP(r); }

bool GeometricPrimitive::intersect(
	const Ray& r, SurfaceIsect* isect) const 
{
	if (!shape->intersect(r, isect)) return false;

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


// Accelerator Method Definitions
const AreaLight* Accelerator::get_AreaLight() const
{
	LOG(FATAL) <<
		"Accelerator::GetAreaLight() method"
		"called; should have gone to GeometricPrimitive";
	return nullptr;
}

const Material* Accelerator::get_material() const
{
	LOG(FATAL) <<
		"Accelerator::GetMaterial() method"
		"called; should have gone to GeometricPrimitive";
	return nullptr;
}

void Accelerator::compute_scattering(
	SurfaceIsect* isect,
//	MemoryArena &arena,
	TransportMode mode,
	bool allowMultipleLobes) const 
{
	LOG(FATAL) <<
		"Accelerator::ComputeScatteringFunctions() method"
		"called; should have gone to GeometricPrimitive";
}

bool Accelerator::intersect(const Ray& r, SurfaceIsect* isect) const
{
	bool flag = false;
	for (auto& p : primitives)
		if (p->intersect(r, isect))
			flag = true;
	return flag ? true : false;
}

bool Accelerator::intersectP(const Ray& r) const
{
	for (auto& p : primitives)
		if (p->intersectP(r))
			return true;
	return false;
}

}	//namespace valley