#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_INTEGRATOR_PAHT_H
#define VALLEY_INTEGRATOR_PATH_H

//#include"valley.h"
#include"integrator.h"

namespace valley
{

class Path : public SamplerIntegrator
{
public:
	Path(std::shared_ptr<Camera> camera, std::shared_ptr<Sampler> sampler,  int maxDepth = 3,
		const Float rrThreshold = 1.f) :
		SamplerIntegrator(camera, sampler, maxDepth), rrThreshold(rrThreshold){}

	void preprocess(const Scene &scene, Sampler &sampler)
	{
		lightDistribution = std::unique_ptr<LightDistribution>{new PowerDistribution(scene)};
	}

	Color Path::Li(const Ray& r, const Scene &scene, Sampler &sampler, int depth) const
	{
		Color L(0.f), beta(1.f);	//beta为 path throughput 项
		Ray ray(r);
		bool specularBounce = false;	//记录最后一个顶点是否有镜面反射
		int bounces;
		// Added after book publication: etaScale tracks the accumulated effect
		// of radiance scaling due to rays passing through refractive
		// boundaries (see the derivation on p. 527 of the third edition). We
		// track this value in order to remove it from beta when we apply
		// Russian roulette; this is worthwhile, since it lets us sometimes
		// avoid terminating refracted rays that are about to be refracted back
		// out of a medium and thus have their beta value increased.
		Float etaScale = 1.f;

		for (bounces = 0;; ++bounces)
		{
			// Find next path vertex and accumulate contribution
			//计算下一个顶点和累计贡献
			DVLOG(2) << "Path tracer bounce " << bounces << ", current L = " << L
				<< ", beta = " << beta;

			// Intersect _ray_ with scene and store intersection in _isect_
			SurfaceIsect isect;
			bool foundIntersection = scene.intersect(ray, &isect);

			// Possibly add emitted light at intersection
			//中间交点的Le会被上一交点的直接光照计算包含在内（但specular项不计入），
			//因而只需要计算最初交点的或最后一个含镜面brdf交点的$L_emit$
			if (bounces == 0 || specularBounce)
			{
				// Add emitted light at path vertex or from the environment
				if (foundIntersection)
				{
					L += beta * isect.Le(-ray.d);
					DVLOG(2) << "Added Le -> L = " << L;
				}
				else
				{
					for (const auto &light : scene.infiniteLights)
						L += beta * light->Le(ray);
					DVLOG(2) << "Added infinite area lights -> L = " << L;
				}
			}

			// Terminate path if ray escaped or _maxDepth_ was reached
			if (!foundIntersection || bounces >= maxDepth) break;

			// Compute scattering functions and skip over medium boundaries
			isect.compute_scattering(ray, TransportMode::Importance, true);
			if (!isect.bsdf)
			{
				DVLOG(2) << "Skipping intersection due to null bsdf";
				ray = isect.generate_ray(ray.d);
				bounces--;
				continue;
			}

			const Distribution1D* distrib = lightDistribution->lookup(isect.p);

			// Sample illumination from lights to find path contribution.
			// (But skip this for perfectly specular BSDFs.)
			//如果只包含specular BSDF，则跳过
			if (isect.bsdf->components_num(
				BxDF_type(static_cast<int>(BxDF_type::All) & ~static_cast<int>(BxDF_type::Specular))) > 0)
			{
				Color Ld = beta * uniform_sample_one_light(isect, scene, //arena,
					sampler, false, distrib);
				DVLOG(2) << "Sampled direct lighting Ld = " << Ld;
				//if (Ld.IsBlack()) ++zeroRadiancePaths;
				CHECK_GE(Ld.luminance(), 0.f);
				L += Ld;
			}

			// Sample BSDF to get new path direction
			Vector3f wo = -ray.d, wi;
			Float pdf;
			BxDF_type flags;
			Color f = isect.bsdf->sample_f(wo, &wi, sampler.get_2D(), &pdf,
				BxDF_type::All, &flags);
			DVLOG(2) << "Sampled BSDF, f = " << f << ", pdf = " << pdf;
			if (f.is_black() || pdf == 0.f) break;
			//更新throughput
			beta *= f * AbsDot(wi, isect.shading.n) / pdf;

			DVLOG(2) << "Updated beta = " << beta;
			CHECK_GE(beta.luminance(), 0.f);
			DCHECK(!std::isinf(beta.luminance()));

			specularBounce = static_cast<int>(flags & BxDF_type::Specular) != 0;
			if (static_cast<int>(flags & BxDF_type::Specular) && 
				static_cast<int>(flags & BxDF_type::Transmission)) 
			{
				Float eta = isect.bsdf->eta;
				// Update the term that tracks radiance scaling for refraction
				// depending on whether the ray is entering or leaving the
				// medium.
				etaScale *= (Dot(wo, isect.n) > 0) ? (eta * eta) : 1 / (eta * eta);
			}
			ray = isect.generate_ray(wi);

			// Account for subsurface scattering, if applicable
			/*
			if (isect.bssrdf && (flags & BSDF_TRANSMISSION)) 
			{
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
			*/

			// Possibly terminate the path with Russian roulette.
			// Factor out radiance scaling due to refraction in rrBeta.
			Color rrBeta = beta * etaScale;
			//3次反射后开始考虑终止路径
			if (rrBeta.max_value() < rrThreshold && bounces > 3)
			{
				Float q = std::max((Float).05, 1 - rrBeta.max_value());	//终止概率
				if (sampler.get() < q) break;
				beta /= 1 - q;
				DCHECK(!std::isinf(beta.luminance()));
			}
		}
		return L;
	}

private:
	//const std::string lightSampleStrategy;
	const Float rrThreshold;	//Russian Roulette Threshold
	std::unique_ptr<LightDistribution> lightDistribution;
};

}	//namespace valley


#endif //VALLEY_INTEGRATOR_PATH_H
