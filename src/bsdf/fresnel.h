#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_BSDF_FOURIER_H
#define RAINY_BSDF_FOURIER_H

#include"rainy.h"
#include"bsdf.h"
#include"spectrum.h"

namespace rainy
{

Float FrDielectric(Float cosThetaI, Float etaI, Float etaT);
Spectrum FrConductor(Float cosThetaI, const Spectrum &etaI,
				  const Spectrum &etaT, const Spectrum &k);

class Fresnel 
{
public:
	virtual ~Fresnel() {}
	virtual Spectrum evaluate(Float cosI) const = 0;
	//virtual std::string ToString() const = 0;
};

class FresnelConductor : public Fresnel 
{
public:
	FresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k)
		: etaI(etaI), etaT(etaT), k(k) {}

	Spectrum evaluate(Float cosThetaI) const override
	{
		return FrConductor(std::abs(cosThetaI), etaI, etaT, k);
	}
	//std::string ToString() const;

private:
	Spectrum etaI, etaT, k;	//etaI和etaT为表面两侧的介质折射率，k为吸收系数
};

class FresnelDielectric : public Fresnel 
{
public:
	FresnelDielectric(Float etaI, Float etaT) : etaI(etaI), etaT(etaT) {}

	Spectrum evaluate(Float cosThetaI) const override
	{
		return FrDielectric(cosThetaI, etaI, etaT);
	}
	//std::string ToString() const;

private:
	Float etaI, etaT;
};

//用在 mirror 中，完全反射所有光线
class FresnelNoOp : public Fresnel
{
public:
	Spectrum evaluate(Float) const override { return Spectrum(1.); }
	//std::string ToString() const { return "[ FresnelNoOp ]"; }
};

}	//namespace rainy


#endif //RAINY_BSDF_FOURIER_H
