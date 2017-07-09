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
	virtual void compute_scattering(SurfaceIsect* isect, TransportMode mode) const = 0;
};


}	//namespace valley


#endif //VALLEY_CORE_PRIMITIVE_H