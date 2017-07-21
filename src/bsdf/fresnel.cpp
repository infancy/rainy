#include"fresnel.h"

namespace valley
{

// Fresnel Utility Functions

//计算绝缘材质的反射率Fr，Ft = 1 - Fr
Float FrDielectric(Float cosThetaI, Float etaI, Float etaT) 
{
	cosThetaI = Clamp(cosThetaI, -1, 1);
	// Potentially swap indices of refraction
	bool entering = cosThetaI > 0.f;
	if (!entering) {
		std::swap(etaI, etaT);
		cosThetaI = std::abs(cosThetaI);
	}

	// Compute _cosThetaT_ using Snell's law
	Float sinThetaI = std::sqrt(std::max((Float)0, 1 - cosThetaI * cosThetaI));
	Float sinThetaT = etaI / etaT * sinThetaI;

	// Handle total internal reflection
	if (sinThetaT >= 1) return 1;
	Float cosThetaT = std::sqrt(std::max((Float)0, 1 - sinThetaT * sinThetaT));

	Float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
				  ((etaT * cosThetaI) + (etaI * cosThetaT));

	Float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
				  ((etaI * cosThetaI) + (etaT * cosThetaT));

	return (Rparl * Rparl + Rperp * Rperp) / 2;
}

// https: //seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
//计算金属等导体的反射率，导体会吸收而不是透射光线，k为吸收率
//待检查
Color FrConductor(Float cosThetaI, const Color &etai,
				  const Color &etat, const Color &k) 
{
	cosThetaI = Clamp(cosThetaI, -1, 1);
	Color eta = etat / etai;
	Color etak = k / etai;

	Float cosThetaI2 = cosThetaI * cosThetaI;
	Float sinThetaI2 = 1. - cosThetaI2;
	Color eta2 = eta * eta;
	Color etak2 = etak * etak;

	Color t0 = eta2 - etak2 - sinThetaI2;
	Color a2plusb2 = (t0 * t0 + 4 * eta2 * etak2).sqrt();
	Color t1 = a2plusb2 + cosThetaI2;
	Color a = (0.5f * (a2plusb2 + t0)).sqrt();
	Color t2 = (Float)2 * cosThetaI * a;
	Color Rs = (t1 - t2) / (t1 + t2);

	Color t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
	Color t4 = t2 * sinThetaI2;
	Color Rp = Rs * (t3 - t4) / (t3 + t4);

	return 0.5 * (Rp + Rs);
}



}	//namespace valley