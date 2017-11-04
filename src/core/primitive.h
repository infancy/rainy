#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_PRIMITIVE_H
#define RAINY_CORE_PRIMITIVE_H

#include"rainy.h"
#include"geometry.h"
#include"material.h"

namespace rainy
{

class Primitive 
{
public:
	virtual ~Primitive() {}

	virtual Bounds3f world_bound() const = 0;
	virtual bool intersect(const Ray &r, SurfaceInteraction*) const = 0;
	virtual bool intersectP(const Ray &r) const = 0;
	virtual const AreaLight* get_AreaLight() const = 0;
	virtual const Material* get_material() const = 0;
	virtual void compute_scattering(SurfaceInteraction* isect, 
								//	MemoryArena &arena,
									TransportMode mode, 
									bool allowMultipleLobes) const = 0;
};

class GeometricPrimitive : public Primitive 
{
public:
	GeometricPrimitive(const std::shared_ptr<Shape>& shape,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<AreaLight>& areaLight = nullptr
		//,const MediumInterface &mediumInterface
		) : 
		shape(shape),
		material(material),
		areaLight(areaLight) 
	  //mediumInterface(mediumInterface)
	{}

	virtual Bounds3f world_bound() const override;

	virtual bool intersect(const Ray& r, SurfaceInteraction* isect) const override;
	virtual bool intersectP(const Ray& r) const override;

	const AreaLight* get_AreaLight() const override;
	const Material* get_material() const override;

	void compute_scattering(SurfaceInteraction* isect,
						//	MemoryArena &arena,
							TransportMode mode,
							bool allowMultipleLobes) const override;

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
	bool Intersect(const Ray &r, SurfaceInteraction *in) const;
	bool IntersectP(const Ray &r) const;
	const AreaLight *GetAreaLight() const { return nullptr; }
	const Material *GetMaterial() const { return nullptr; }
	void compute_scattering(SurfaceInteraction *isect,
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

class Accelerator : public Primitive
{
public:
	Accelerator(std::vector<std::shared_ptr<Primitive>>& primitives) :
		primitives(primitives)
	{
		//创建加速体的包围盒
		for (auto& p : primitives)
			bounds = Union(bounds, p->world_bound());
	}
	virtual Bounds3f world_bound() const override { return bounds; }

	const AreaLight* get_AreaLight() const override;
	const Material* get_material() const override;

	void compute_scattering(SurfaceInteraction* isect, 
						//	MemoryArena &arena,
							TransportMode mode,
							bool allowMultipleLobes) const override;

	virtual bool intersect(const Ray& r, SurfaceInteraction* isect) const override;
	virtual bool intersectP(const Ray& r) const override;

private:
	std::vector<std::shared_ptr<Primitive>> primitives;
	Bounds3f bounds;
};

}	//namespace rainy


#endif //RAINY_CORE_PRIMITIVE_H
