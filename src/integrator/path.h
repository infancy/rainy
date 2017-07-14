#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_INTEGRATOR_PAHT_H
#define VALLEY_INTEGRATOR_PATH_H

#include"valley.h"
#include"integrator.h"
//#include <glog/logging.h>

namespace valley
{

class Path : public Integrator
{
public:
	Path(std::shared_ptr<Camera> camera, int seed = 1234, int maxDepth = 3) :
		Integrator(camera, seed), maxDepth(maxDepth) {}
	/*
	virtual void render(const Scene& scene)
	{
		++iterations;

		//对所有灯光进行均匀采样
		const int   lightCount = scene.lights.size();
		const float lightPickProb = 1.f / lightCount;

		for (int y = 0; y < camera->film->height; ++y)
		{
			static const Float kEpsilon = 0.001f;

			for (int x = 0; x < camera->film->width; ++x)
			{
				CameraSample cs;
				cs.pFilm = Point2f(x, y) + rng.get_v2f();

				Ray ray;
				camera->generate_ray(cs, &ray);
				SurfaceIsect isect;

				//cout << "x: " << x << " y: " << y << "\n";

				Point3f   pathWeight(1.f, 1.f, 1.f);
				Color	  color(0.f);
				int64_t   pathLength = 1;
				bool      lastSpecular = true;
				float     lastPdfW = 1;

			while(++pathLength)
			{
				if (!scene.intersect(ray, &isect))
				{
					if (pathLength < min_PathLength)
						break;
					
					const BackgroundLight* background = mScene.GetBackground();
					if (!background)
						break;

					// For background we cheat with the A/W suffixes,
					// and GetRadiance actually returns W instead of A
					float directPdfW;
					Vec3f contrib = background->GetRadiance(mScene.mSceneSphere,
					ray.dir, Vec3f(0), &directPdfW);
					if (contrib.IsZero())
						break;

					float misWeight = 1.f;
					if (pathLength > 1 && !lastSpecular)
					{
						misWeight = mis2(lastPdfW, directPdfW * lightPickProb);
					}

					color += pathWeight * misWeight * contrib;
					break;
				}
				
				
				Point3f hitPoint = ray.o + ray.d * ray.tMax;
				ray.tMax += kEpsilon;

				BSDF<false> bsdf(ray, isect, mScene);
				if (!bsdf.IsValid())
					break;

				// directly hit some light, lights do not reflect
				if (isect.lightID >= 0)
				{
					if (pathLength < mMinPathLength)
						break;

					const AbstractLight *light = mScene.GetLightPtr(isect.lightID);
					float directPdfA;
					Vec3f contrib = light->GetRadiance(mScene.mSceneSphere,
						ray.dir, hitPoint, &directPdfA);
					if (contrib.IsZero())
						break;

					float misWeight = 1.f;
					if (pathLength > 1 && !lastSpecular)
					{
						const float directPdfW = PdfAtoW(directPdfA, isect.dist,
							bsdf.CosThetaFix());
						misWeight = Mis2(lastPdfW, directPdfW * lightPickProb);
					}

					color += pathWeight * misWeight * contrib;
					break;
				}
				
				if (pathLength >= mMaxPathLength)
					break;

				if (bsdf.ContinuationProb() == 0)
					break;

				// next event estimation
				if (!bsdf.IsDelta() && pathLength + 1 >= mMinPathLength)
				{
					int lightID = int(mRng.GetFloat() * lightCount);
					const AbstractLight *light = mScene.GetLightPtr(lightID);

					Vec3f directionToLight;
					float distance, directPdfW;
					Vec3f radiance = light->Illuminate(mScene.mSceneSphere, hitPoint,
						mRng.GetVec2f(), directionToLight, distance, directPdfW);

					if (!radiance.IsZero())
					{
						float bsdfPdfW, cosThetaOut;
						const Vec3f factor = bsdf.Evaluate(mScene,
							directionToLight, cosThetaOut, &bsdfPdfW);

						if (!factor.IsZero())
						{
							float weight = 1.f;
							if (!light->IsDelta())
							{
								const float contProb = bsdf.ContinuationProb();
								bsdfPdfW *= contProb;
								weight = Mis2(directPdfW * lightPickProb, bsdfPdfW);
							}

							Vec3f contrib = (weight * cosThetaOut / (lightPickProb * directPdfW)) *
								(radiance * factor);

							if (!mScene.Occluded(hitPoint, directionToLight, distance))
							{
								color += pathWeight * contrib;
							}
						}
					}
				}

				// continue random walk
				{
					Vec3f rndTriplet = mRng.GetVec3f();
					float pdf, cosThetaOut;
					uint  sampledEvent;

					Vec3f factor = bsdf.Sample(mScene, rndTriplet, ray.dir,
						pdf, cosThetaOut, &sampledEvent);

					if (factor.IsZero())
						break;

					// Russian roulette
					const float contProb = bsdf.ContinuationProb();

					lastSpecular = (sampledEvent & BSDF<true>::kSpecular) != 0;
					lastPdfW = pdf * contProb;

					if (contProb < 1.f)
					{
						if (mRng.GetFloat() > contProb)
						{
							break;
						}
						pdf *= contProb;
					}

					pathWeight *= factor * (cosThetaOut / pdf);
					// We offset ray origin instead of setting tmin due to numeric
					// issues in ray-sphere intersection. The isect.dist has to be
					// extended by this EPS_RAY after hitpoint is determined
					ray.org = hitPoint + EPS_RAY * ray.dir;
					ray.tmin = 0.f;
					isect.dist = 1e36f;
				}
			
			}
			}
			camera->film->add(x, y, color);
		}
	}
	*/

Color Path::Li(const RayDifferential &r, const Scene &scene,
	Sampler &sampler, //MemoryArena &arena,
	int depth) const
{
	Color L(0.f), beta(1.f);
	RayDifferential ray(r);
	bool specularBounce = false;
	int bounces;
	// Added after book publication: etaScale tracks the accumulated effect
	// of radiance scaling due to rays passing through refractive
	// boundaries (see the derivation on p. 527 of the third edition). We
	// track this value in order to remove it from beta when we apply
	// Russian roulette; this is worthwhile, since it lets us sometimes
	// avoid terminating refracted rays that are about to be refracted back
	// out of a medium and thus have their beta value increased.
	Float etaScale = 1;

	for (bounces = 0;; ++bounces) 
	{
		// Find next path vertex and accumulate contribution

		// Intersect _ray_ with scene and store intersection in _isect_
		SurfaceIsect isect;
		bool foundIntersection = scene.intersect(ray, &isect);

		// Possibly add emitted light at intersection
		if (bounces == 0 || specularBounce)
		{
			// Add emitted light at path vertex or from the environment
			if (foundIntersection) 
			{
				L += beta * isect.Le(-ray.d);
				//VLOG(2) << "Added Le -> L = " << L;
			}
			else 
			{
				for (const auto &light : scene.infiniteLights)
					L += beta * light->Le(ray);
				//VLOG(2) << "Added infinite area lights -> L = " << L;
			}
		}

		// Terminate path if ray escaped or _maxDepth_ was reached
		if (!foundIntersection || bounces >= maxDepth) break;

		// Compute scattering functions and skip over medium boundaries
		isect.compute_scattering(ray, TransportMode::Importance, true);
		if (!isect.bsdf) 
		{
			//VLOG(2) << "Skipping intersection due to null bsdf";
			ray = isect.generate_ray(ray.d);
			bounces--;
			continue;
		}

		const Distribution1D* distrib = lightDistribution->Lookup(isect.p);

		// Sample illumination from lights to find path contribution.
		// (But skip this for perfectly specular BSDFs.)
		if (isect.bsdf->components_num(
			BxDF_type(static_cast<int>(BxDF_type::All) & ~static_cast<int>(BxDF_type::Specular))) > 0)
		{
			Color Ld = beta * UniformSampleOneLight(isect, scene, //arena,
				sampler, false, distrib);
			//VLOG(2) << "Sampled direct lighting Ld = " << Ld;
			//if (Ld.IsBlack()) ++zeroRadiancePaths;
			CHECK_GE(Ld.luminance(), 0.f);
			L += Ld;
		}

		// Sample BSDF to get new path direction
		Vector3f wo = -ray.d, wi;
		Float pdf;
		BxDF_type flags;
		Color f = isect.bsdf->sample_f(wo, &wi, sampler.Get2D(), &pdf,
			BxDF_type::All, &flags);
		//VLOG(2) << "Sampled BSDF, f = " << f << ", pdf = " << pdf;
		if (f.IsBlack() || pdf == 0.f) break;
		beta *= f * AbsDot(wi, isect.shading.n) / pdf;
		//VLOG(2) << "Updated beta = " << beta;
		CHECK_GE(beta.y(), 0.f);
		DCHECK(!std::isinf(beta.y()));
		specularBounce = (flags & BSDF_SPECULAR) != 0;
		if ((flags & BSDF_SPECULAR) && (flags & BSDF_TRANSMISSION)) {
			Float eta = isect.bsdf->eta;
			// Update the term that tracks radiance scaling for refraction
			// depending on whether the ray is entering or leaving the
			// medium.
			etaScale *= (Dot(wo, isect.n) > 0) ? (eta * eta) : 1 / (eta * eta);
		}
		ray = isect.SpawnRay(wi);

		// Account for subsurface scattering, if applicable
		if (isect.bssrdf && (flags & BSDF_TRANSMISSION)) {
			// Importance sample the BSSRDF
			SurfaceInteraction pi;
			Color S = isect.bssrdf->Sample_S(
				scene, sampler.Get1D(), sampler.Get2D(), arena, &pi, &pdf);
			DCHECK(!std::isinf(beta.y()));
			if (S.IsBlack() || pdf == 0) break;
			beta *= S / pdf;

			// Account for the direct subsurface scattering component
			L += beta * UniformSampleOneLight(pi, scene, arena, sampler, false,
				lightDistribution->Lookup(pi.p));

			// Account for the indirect subsurface scattering component
			Color f = pi.bsdf->Sample_f(pi.wo, &wi, sampler.Get2D(), &pdf,
				BSDF_ALL, &flags);
			if (f.IsBlack() || pdf == 0) break;
			beta *= f * AbsDot(wi, pi.shading.n) / pdf;
			DCHECK(!std::isinf(beta.y()));
			specularBounce = (flags & BSDF_SPECULAR) != 0;
			ray = pi.SpawnRay(wi);
		}

		// Possibly terminate the path with Russian roulette.
		// Factor out radiance scaling due to refraction in rrBeta.
		Color rrBeta = beta * etaScale;
		if (rrBeta.MaxComponentValue() < rrThreshold && bounces > 3) {
			Float q = std::max((Float).05, 1 - rrBeta.MaxComponentValue());
			if (sampler.Get1D() < q) break;
			beta /= 1 - q;
			DCHECK(!std::isinf(beta.luminance()));
		}
	}
	return L;
}

private:
	// Mis power (1 for balance heuristic)
	Float mis(Float pdf) const { return pdf; }

	// Mis weight for 2 pdfs
	Float mis2( Float aSamplePdf, Float aOtherPdf) const
	{
		return mis(aSamplePdf) / (mis(aSamplePdf) + mis(aOtherPdf));
	}

private:
	const int maxDepth;
	const std::string lightSampleStrategy;
	//std::unique_ptr<LightDistribution> lightDistribution;
};

}	//namespace valley


#endif //VALLEY_INTEGRATOR_PATH_H
