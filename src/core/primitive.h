#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_PRIMITIVE_H
#define VALLEY_CORE_PRIMITIVE_H

#include"valley.h"
#include"geometry.h"
#include"material.h"

namespace valley
{

class Primitive 
{
public:
	virtual ~Primitive();		//¼ì²é

	virtual Bounds3f WorldBound() const = 0;
	virtual bool Intersect(const Ray &r, SurfaceIsect*) const = 0;
	virtual bool IntersectP(const Ray &r) const = 0;
	virtual const AreaLight* get_AreaLight() const = 0;
	virtual const Material* get_material() const = 0;
	virtual void compute_scattering(SurfaceIsect* isect, TransportMode mode, 
									bool allowMultipleLobes) const = 0;
};

class GeometricPrimitive : public Primitive 
{
public:
	GeometricPrimitive(const std::shared_ptr<Shape> &shape,
		const std::shared_ptr<Material> &material,
		const std::shared_ptr<AreaLight> &areaLight) : 
		shape(shape),
		material(material),
		areaLight(areaLight) {}

	virtual Bounds3f WorldBound() const;
	virtual bool Intersect(const Ray& r, SurfaceIsect* isect) const;
	virtual bool IntersectP(const Ray& r) const;
	const AreaLight* GetAreaLight() const;
	const Material* GetMaterial() const;
	void compute_scattering(SurfaceIsect* isect,
							TransportMode mode,
							bool allowMultipleLobes) const override;

private:
	// GeometricPrimitive Private Data
	std::shared_ptr<Shape> shape;
	std::shared_ptr<Material> material;
	std::shared_ptr<AreaLight> areaLight;
	//MediumInterface mediumInterface;
};

}	//namespace valley


#endif //VALLEY_CORE_PRIMITIVE_H