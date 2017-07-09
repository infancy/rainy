#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_LAMBERTIAN_H
#define VALLEY_CORE_LAMBERTIAN_H

#include"valley.h"
#include"bsdf.h"
#include"color.h"

namespace valley
{

class LambertianReflection : public BxDF 
{
public:
	// LambertianReflection Public Methods
	LambertianReflection(const Color4f& R)
		: BxDF(BxDF_type(BxDF_type::reflection | BxDF_type::diffuse)), R(R) {}

	Color4f f(const Vector3f& wo, const Vector3f& wi) const { return R * InvPi; }
	Color4f rho(const Vector3f&, int, const Point2f*) const { return R; }
	Color4f rho(int, const Point2f*, const Point2f*) const  { return R; }
	//std::string ToString() const;

private:
	// LambertianReflection Private Data
	const Color4f R;
};

class LambertianTransmission : public BxDF {
public:
	// LambertianTransmission Public Methods
	LambertianTransmission(const Color4f& T)
		: BxDF(BxDF_type(BxDF_type::transmission | BxDF_type::diffuse)), T(T) {}

	Color4f f(const Vector3f& wo, const Vector3f& wi) const { return T * InvPi; }
	Color4f rho(const Vector3f&, int, const Point2f*) const { return T; }
	Color4f rho(int, const Point2f*, const Point2f*) const  { return T; }

	Color4f sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u,
					 Float* pdf, BxDF_type* sampledType) const;
	Float pdf(const Vector3f& wo, const Vector3f& wi) const;
	//std::string ToString() const;

private:
	// LambertianTransmission Private Data
	Color4f T;
};

}	//namespace valley


#endif //VALLEY_CORE_LAMBERTIAN_H
