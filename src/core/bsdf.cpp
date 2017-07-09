/*
pbrt source code is Copyright(c) 1998-2016
Matt Pharr, Greg Humphreys, and Wenzel Jakob.

This file is part of pbrt.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include"bsdf.h"
#include"color.h"
#include"rng.h"
#include"sampling.h"
#include"intersection.h"

namespace valley
{

// BxDF Method Definitions

Color4f BxDF::sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u,
					   Float* Pdf, BxDF_type* sampledType) const
{
	// Cosine-sample the hemisphere, flipping the direction if necessary
	*wi = cosine_sample_hemisphere(u);
	if (wo.z < 0) wi->z *= -1;
	*Pdf = pdf(wo, *wi);
	return f(wo, *wi);
}

Color4f BxDF::rho(const Vector3f& w, int nSamples, const Point2f* u) const {
	Color4f r(0.);
	for (int i = 0; i < nSamples; ++i) {
		// Estimate one term of $\rho_\roman{hd}$
		Vector3f wi;
		Float pdf = 0;
		Color4f f = sample_f(w, &wi, u[i], &pdf);
		if (pdf > 0) r += f * AbsCosTheta(wi) / pdf;
	}
	return r / nSamples;
}

Color4f BxDF::rho(int nSamples, const Point2f* u1, const Point2f* u2) const {
	Color4f r(0.f);
	for (int i = 0; i < nSamples; ++i) {
		// Estimate one term of $\rho_\roman{hh}$
		Vector3f wo, wi;
		wo = uniform_sample_hemisphere(u1[i]);
		Float pdfo = uniform_hemisphere_pdf(), pdfi = 0;
		Color4f f = sample_f(wo, &wi, u2[i], &pdfi);
		if (pdfi > 0)
			r += f * AbsCosTheta(wi) * AbsCosTheta(wo) / (pdfo * pdfi);
	}
	return r / (Pi * nSamples);
}

Float BxDF::pdf(const Vector3f &wo, const Vector3f &wi) const 
{
	return same_hemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0;
}


// BSDF Method Definitions

BSDF::BSDF(const SurfaceIsect& si, Float eta) :
	eta(eta),
	ns(si.shading.n),
	ng(si.n),
	ss(Normalize(si.shading.dpdu)),
	ts(Cross(ns, ss)) {}

int BSDF::components_num(BxDF_type flags) const
{
	int num = 0;
	for (int i = 0; i < nBxDFs; ++i)
		if (bxdfs[i]->match(flags)) ++num;
	return num;
}

Color4f BSDF::f(const Vector3f &woW, const Vector3f &wiW,
				BxDF_type flags) const 
{
	Vector3f wi = world_to_local(wiW), wo = world_to_local(woW);
	if (wo.z == 0.f) return 0.f;

	bool reflect = Dot(wiW, ng) * Dot(woW, ng) > 0;	//处理着色法线的缺陷
	Color4f f(0.f);

	for (int i = 0; i < nBxDFs; ++i)
		if (bxdfs[i]->match(flags) &&
		    ((reflect && static_cast<bool>(bxdfs[i]->type & BxDF_type::Reflection)) ||
			(!reflect && static_cast<bool>(bxdfs[i]->type & BxDF_type::Transmission))))
			f += bxdfs[i]->f(wo, wi);
	return f;
}

Color4f BSDF::sample_f(const Vector3f& woWorld, Vector3f* wiWorld,
					   const Point2f& u, Float *pdf, BxDF_type type,
					   BxDF_type* sampledType) const
{

	// Choose which _BxDF_ to sample
	int matchingComps = components_num(type);
	if (matchingComps == 0) {
		*pdf = 0;
		if (sampledType) *sampledType = BxDF_type(0);
		return Color4f(0);
	}
	int comp =
		std::min((int)std::floor(u[0] * matchingComps), matchingComps - 1);

	// Get _BxDF_ pointer for chosen component
	BxDF* bxdf = nullptr;
	int count = comp;
	for (int i = 0; i < nBxDFs; ++i)
		if (bxdfs[i]->match(type) && count-- == 0) {
			bxdf = bxdfs[i];
			break;
		}
	CHECK_NOTNULL(bxdf);
	/*
	VLOG(2) << "BSDF::Sample_f chose comp = " << comp << " / matching = " <<
	matchingComps << ", bxdf: " << bxdf->ToString();
	*/

	// Remap _BxDF_ sample _u_ to $[0,1)^2$
	Point2f uRemapped(std::min(u[0] * matchingComps - comp, OneMinusEpsilon), u[1]);

	// Sample chosen _BxDF_
	Vector3f wi, wo = world_to_local(woWorld);
	if (wo.z == 0) return 0.;
	*pdf = 0;
	if (sampledType) *sampledType = bxdf->type;
	Color4f f = bxdf->sample_f(wo, &wi, uRemapped, pdf, sampledType);
	/*
	VLOG(2) << "For wo = " << wo << ", sampled f = " << f << ", pdf = "
	<< *pdf << ", ratio = " << ((*pdf > 0) ? (f / *pdf) : Color4f(0.))
	<< ", wi = " << wi;
	*/
	if (*pdf == 0) {
		if (sampledType) *sampledType = BxDF_type(0);
		return 0;
	}
	*wiWorld = local_to_world(wi);

	// Compute overall PDF with all matching _BxDF_s
	if (!static_cast<bool>(bxdf->type & BxDF_type::Specular) && matchingComps > 1)
		for (int i = 0; i < nBxDFs; ++i)
			if (bxdfs[i] != bxdf && bxdfs[i]->match(type))
				*pdf += bxdfs[i]->pdf(wo, wi);
	if (matchingComps > 1) *pdf /= matchingComps;

	// Compute value of BSDF for sampled direction
	if (!static_cast<bool>(bxdf->type & BxDF_type::Specular) && matchingComps > 1) {
		bool reflect = Dot(*wiWorld, ng) * Dot(woWorld, ng) > 0;
		f = 0.;
		for (int i = 0; i < nBxDFs; ++i)
			if (bxdfs[i]->match(type) &&
				((reflect && static_cast<bool>(bxdfs[i]->type & BxDF_type::Reflection)) ||
				(!reflect && static_cast<bool>(bxdfs[i]->type & BxDF_type::Transmission))))
				f += bxdfs[i]->f(wo, wi);
	}
	/*
	VLOG(2) << "Overall f = " << f << ", pdf = " << *pdf << ", ratio = "
	<< ((*pdf > 0) ? (f / *pdf) : Color4f(0.));
	*/
	return f;
}

Color4f BSDF::rho(int nSamples, const Point2f* samples1, const Point2f* samples2,
				  BxDF_type flags) const
{
	Color4f ret(0.f);
	for (int i = 0; i < nBxDFs; ++i)
		if (bxdfs[i]->match(flags))
			ret += bxdfs[i]->rho(nSamples, samples1, samples2);
	return ret;
}

Color4f BSDF::rho(const Vector3f& wo, int nSamples, const Point2f* samples,
				  BxDF_type flags) const 
{
	Color4f ret(0.f);
	for (int i = 0; i < nBxDFs; ++i)
		if (bxdfs[i]->match(flags))
			ret += bxdfs[i]->rho(wo, nSamples, samples);
	return ret;
}

Float BSDF::pdf(const Vector3f &woWorld, const Vector3f &wiWorld,
	BxDF_type flags) const 
{
	if (nBxDFs == 0.f) return 0.f;
	Vector3f wo = world_to_local(woWorld), wi = world_to_local(wiWorld);
	if (wo.z == 0) return 0.;
	Float pdf = 0.f;
	int matchingComps = 0;
	for (int i = 0; i < nBxDFs; ++i)
		if (bxdfs[i]->match(flags)) {
			++matchingComps;
			pdf += bxdfs[i]->pdf(wo, wi);
		}
	Float v = matchingComps > 0 ? pdf / matchingComps : 0.f;
	return v;
}

}	//namespace valley