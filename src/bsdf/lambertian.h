#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_LAMBERTIAN_H
#define RAINY_CORE_LAMBERTIAN_H

#include"rainy.h"
#include"bsdf.h"
#include"spectrum.h"

namespace rainy
{

class LambertianReflection : public BxDF 
{
public:
	// LambertianReflection Public Methods
	LambertianReflection(const Spectrum& R)
		: BxDF(BxDFType(BxDFType::Reflection | BxDFType::Diffuse)), R(R) {}

	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override { return R * InvPi; }
	Spectrum rho(const Vector3f&, int, const Point2f*) const { return R; }
	Spectrum rho(int, const Point2f*, const Point2f*) const  { return R; }
	//std::string ToString() const;

private:
	// LambertianReflection Private Data
	const Spectrum R;
};

class LambertianTransmission : public BxDF {
public:
	// LambertianTransmission Public Methods
	LambertianTransmission(const Spectrum& T)
		: BxDF(BxDFType(BxDFType::Transmission | BxDFType::Diffuse)), T(T) {}

	Spectrum f(const Vector3f& wo, const Vector3f& wi) const override { return T * InvPi; }
	Spectrum rho(const Vector3f&, int, const Point2f*) const { return T; }
	Spectrum rho(int, const Point2f*, const Point2f*) const  { return T; }

	Spectrum sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u,
					 Float* pdf, BxDFType* sampledType) const;
	Float pdf(const Vector3f& wo, const Vector3f& wi) const;
	//std::string ToString() const;

private:
	// LambertianTransmission Private Data
	Spectrum T;
};

}	//namespace rainy


#endif //RAINY_CORE_LAMBERTIAN_H
