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
	virtual ~Primitive() {}

	virtual Bounds3f WorldBound() const = 0;
	virtual bool intersect(const Ray &r, SurfaceIsect*) const = 0;
	virtual bool intersectP(const Ray &r) const = 0;
	virtual const AreaLight* get_AreaLight() const = 0;
	virtual const Material* get_material() const = 0;
	virtual void compute_scattering(SurfaceIsect* isect, 
								//	MemoryArena &arena,
									TransportMode mode, 
									bool allowMultipleLobes) const = 0;
};

class GeometricPrimitive : public Primitive 
{
public:
	GeometricPrimitive(const std::shared_ptr<Shape> &shape,
		const std::shared_ptr<Material> &material,
		const std::shared_ptr<AreaLight> &areaLight
		//,const MediumInterface &mediumInterface
		) : 
		shape(shape),
		material(material),
		areaLight(areaLight) 
	  //mediumInterface(mediumInterface)
	{}

	virtual Bounds3f WorldBound() const;

	virtual bool intersect(const Ray& r, SurfaceIsect* isect) const;
	virtual bool intersectP(const Ray& r) const;

	const AreaLight* get_AreaLight() const;
	const Material* get_material() const;

	void compute_scattering(SurfaceIsect* isect,
						//	MemoryArena &arena,
							TransportMode mode,
							bool allowMultipleLobes) const;

private:
	std::shared_ptr<Shape> shape;
	std::shared_ptr<Material> material;
	std::shared_ptr<AreaLight> areaLight;
	//MediumInterface mediumInterface;
};

class Aggregate : public Primitive 
{
public:
	const AreaLight* get_AreaLight() const;
	const Material* get_material() const;

	void compute_scattering(SurfaceIsect* isect, 
						//	MemoryArena &arena,
							TransportMode mode,
							bool allowMultipleLobes) const;
};

}	//namespace valley


#endif //VALLEY_CORE_PRIMITIVE_H