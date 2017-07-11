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

	virtual Bounds3f world_bound() const = 0;
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

	virtual Bounds3f world_bound() const;

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

//处理大量相同实例
/*
class TransformedPrimitive : public Primitive
{
public:
	// TransformedPrimitive Public Methods
	TransformedPrimitive(std::shared_ptr<Primitive> &primitive,
		const AnimatedTransform &PrimitiveToWorld)
		: primitive(primitive), PrimitiveToWorld(PrimitiveToWorld) {}
	bool Intersect(const Ray &r, SurfaceIsect *in) const;
	bool IntersectP(const Ray &r) const;
	const AreaLight *GetAreaLight() const { return nullptr; }
	const Material *GetMaterial() const { return nullptr; }
	void compute_scattering(SurfaceIsect *isect,
		//MemoryArena &arena, 
		TransportMode mode,
		bool allowMultipleLobes) const {
		LOG(FATAL) <<
			"TransformedPrimitive::ComputeScatteringFunctions() shouldn't be "
			"called";
	}
	Bounds3f world_bound() const {
		return PrimitiveToWorld.MotionBounds(primitive->world_bound());
	}

private:
	// TransformedPrimitive Private Data
	std::shared_ptr<Primitive> primitive;
	const AnimatedTransform PrimitiveToWorld;
};
*/

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