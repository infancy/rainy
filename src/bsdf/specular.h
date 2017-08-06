#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_BSDF_SPECULAR_H
#define VALLEY_BSDF_SPECULAR_H

#include"valley.h"
#include"bsdf.h"
#include"fresnel.h"

namespace valley
{

inline Vector3f Reflect(const Vector3f &wo, const Vector3f &n) {
	return -wo + 2 * Dot(wo, n) * n;
}

inline bool Refract(const Vector3f &wi, const Normal3f &n, Float eta, Vector3f *wt) 
{
	// Compute $\cos \theta_\roman{t}$ using Snell's law
	Float cosThetaI = Dot(n, wi);
	Float sin2ThetaI = std::max(Float(0), Float(1 - cosThetaI * cosThetaI));
	Float sin2ThetaT = eta * eta * sin2ThetaI;

	// Handle total internal reflection for transmission
	//wi接近于水平入射，不发生折射
	if (sin2ThetaT >= 1) return false;
	Float cosThetaT = std::sqrt(1 - sin2ThetaT);
	*wt = eta * -wi + (eta * cosThetaI - cosThetaT) * Vector3f(n);
	return true;
}

class SpecularReflection : public BxDF 
{
public:
	SpecularReflection(const Spectrum &R, Fresnel *fresnel)
		: BxDF(BxDFType::Reflection | BxDFType::Specular),
		R(R), fresnel(fresnel) {}

	Spectrum f(const Vector3f &wo, const Vector3f &wi) const override { return Spectrum(0.f); }

	Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
				   Float *pdf, BxDFType*sampledType) const
	{
		// Compute perfect specular reflection direction
		*wi = Vector3f(-wo.x, -wo.y, wo.z);
		*pdf = 1;

		return fresnel->evaluate(CosTheta(*wi)) * R / AbsCosTheta(*wi);
		//在 mirror 中 return 1.f * R / AbsCosTheta(*wi);
	}

	Float pdf(const Vector3f &wo, const Vector3f &wi) const { return 0; }

	//std::string ToString() const;

private:
	const Spectrum R;
	const std::unique_ptr<Fresnel> fresnel;
};

class SpecularTransmission : public BxDF 
{
public:
	SpecularTransmission(const Spectrum &T, Float etaA, Float etaB,
		TransportMode mode)
		: BxDF(BxDFType(BxDFType::Transmission| BxDFType::Specular)),
		T(T), etaA(etaA), etaB(etaB),
		fresnel(etaA, etaB), mode(mode) {}

	Spectrum f(const Vector3f &wo, const Vector3f &wi) const { return Spectrum(0.f); }

	Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
		Float *pdf, BxDFType*sampledType) const
	{
		// Figure out which $\eta$ is incident and which is transmitted
		bool entering = CosTheta(wo) > 0;
		Float etaI = entering ? etaA : etaB;
		Float etaT = entering ? etaB : etaA;

		// Compute ray direction for specular transmission
		if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
			return 0;
		*pdf = 1;
		Spectrum ft = T * (Spectrum(1.) - fresnel.evaluate(CosTheta(*wi)));
		// Account for non-symmetry with transmission to different medium
		if (mode == TransportMode::Radiance) ft *= (etaI * etaI) / (etaT * etaT);
		return ft / AbsCosTheta(*wi);
	}

	Float pdf(const Vector3f &wo, const Vector3f &wi) const { return 0; }

	//std::string ToString() const;

private:
	const Spectrum T;
	const Float etaA, etaB;
	const FresnelDielectric fresnel;
	const TransportMode mode;
};

class FresnelSpecular : public BxDF
{
public:
	FresnelSpecular(const Spectrum &R, const Spectrum &T, Float etaA,
		Float etaB, TransportMode mode)
		: BxDF(BxDFType(BxDFType::Reflection | BxDFType::Transmission | BxDFType::Specular)),
		R(R), T(T), etaA(etaA), etaB(etaB), mode(mode) {}

	Spectrum f(const Vector3f &wo, const Vector3f &wi) const { return Spectrum(0.f); }

	Spectrum sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
		Float *pdf, BxDFType*sampledType) const
	{
		//根据反射率来确定是计算反射还是折射
		Float F = FrDielectric(CosTheta(wo), etaA, etaB);
		if (u[0] < F) 
		{
			// Compute specular reflection for _FresnelSpecular_

			// Compute perfect specular reflection direction
			*wi = Vector3f(-wo.x, -wo.y, wo.z);
			if (sampledType)
				*sampledType = BxDFType::Specular | BxDFType::Reflection;
			*pdf = F;
			return F * R / AbsCosTheta(*wi);
		}
		else 
		{
			// Compute specular transmission for _FresnelSpecular_

			// Figure out which $\eta$ is incident and which is transmitted
			bool entering = CosTheta(wo) > 0;
			Float etaI = entering ? etaA : etaB;
			Float etaT = entering ? etaB : etaA;

			// Compute ray direction for specular transmission
			if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
				return 0;
			Spectrum ft = T * (1 - F);

			// Account for non-symmetry with transmission to different medium
			if (mode == TransportMode::Radiance)
				ft *= (etaI * etaI) / (etaT * etaT);
			if (sampledType)
				*sampledType = BxDFType::Specular | BxDFType::Transmission;
			*pdf = 1 - F;
			return ft / AbsCosTheta(*wi);
		}
	}

	Float pdf(const Vector3f &wo, const Vector3f &wi) const { return 0; }

	//std::string ToString() const;

private:
	const Spectrum R, T;
	const Float etaA, etaB;
	const TransportMode mode;
};

}	//namespace valley


#endif //VALLEY_BSDF_SPECULAR_H
