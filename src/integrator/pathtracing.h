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

class PathTracing : public SamplerIntegrator
{
public:
	PathTracing(const Scene &scene, std::shared_ptr<Camera> camera, std::shared_ptr<Sampler> sampler,  
		int maxDepth = 3, const Float rrThreshold = 1.f) :
		SamplerIntegrator(camera, sampler), maxDepth(maxDepth), rrThreshold(rrThreshold)
	{
		lightDistribution = std::unique_ptr<LightDistribution>{ new PowerDistribution(scene) };
	}

	Spectrum Li(const Ray& r, const Scene& scene, Sampler& sampler, int depth) const
	{
		Spectrum L(0.f), beta(1.f);	// beta 为 path throughput 项
		Ray ray(r);
		bool specularBounce = false;	// 记录最后一个顶点是否存在镜面材质
		int bounces;
		// etaScale 跟踪光线经由折射产生的累计辐射放缩效果
		// 这样我们在应用俄罗斯轮盘赌时就可以将其从 beta 项中移除
		// 这使我们可以避免终止被集中折射出 media 的光线以及因此引起的 beta 项的增加
		Float etaScale = 1.f;	

		for (bounces = 0;; ++bounces)
		{
			// Find next path vertex and accumulate contribution
			// 计算下一个顶点和累计贡献
			VLOG(2) << "Path tracer bounce " << bounces << ", current L = " << L
				<< ", beta = " << beta;

			// Intersect _ray_ with scene and store intersection in _isect_
			SurfaceInteraction isect;
			bool foundIntersection = scene.intersect(ray, &isect);

			// Possibly add emitted light at intersection
			// 中间交点的 Le 项会被上一交点的直接光照计算包含在内（但镜面项不计入）
			// 因而只需要计算最初交点或最后一个含镜面 bsdf 交点的 Le
			if (bounces == 0 || specularBounce)
			{
				if (foundIntersection)
				{
					L += beta * isect.Le(-ray.d);
					VLOG(2) << "Added Le -> L = " << L;
				}
				else
				{
					for (const auto &light : scene.infiniteLights)
						L += beta * light->Le(ray);
					VLOG(2) << "Added infinite area lights -> L = " << L;
				}
			}

			if (!foundIntersection || bounces >= maxDepth) break;

			isect.compute_scattering(ray, TransportMode::Importance, true);
			if (!isect.bsdf)
			{
				VLOG(2) << "Skipping intersection due to null bsdf";
				ray = isect.generate_ray(ray.d);
				bounces--;
				continue;
			}

			const Distribution1D* distrib = lightDistribution->lookup(isect.p);

			// Sample illumination from lights to find path contribution.
			// (But skip this for perfectly specular BSDFs.)
			//不能直接在镜面 BSDF上计算着色，跳过
			if (isect.bsdf->components_num(
				BxDFType(static_cast<int>(BxDFType::All) & ~static_cast<int>(BxDFType::Specular))) > 0)
			{
				Spectrum Ld = beta * uniform_sample_one_light(isect, scene, //arena,
					sampler, false, distrib);
				VLOG(2) << "Sampled direct lighting Ld = " << Ld;
				//if (Ld.IsBlack()) ++zeroRadiancePaths;
				CHECK_GE(Ld.luminance(), 0.f);
				L += Ld;
			}

			// 对 BSDF 进行采样以生成新的路径方向（即生成下一条光线）
			Vector3f wo = -ray.d, wi;
			Float pdf;
			BxDFType flags;
			Spectrum f = isect.bsdf->sample_f(wo, &wi, sampler.get_2D(), &pdf,
				BxDFType::All, &flags);
			VLOG(2) << "Sampled BSDF, f = " << f << ", pdf = " << pdf;
			if (f.is_black() || pdf == 0.f) break;
			// 更新路径吞吐项
			beta *= f * AbsDot(wi, isect.shading.n) / pdf;

			VLOG(2) << "Updated beta = " << beta;
			CHECK_GE(beta.luminance(), 0.f);
			DCHECK(!std::isinf(beta.luminance()));

			specularBounce = static_cast<int>(flags & BxDFType::Specular) != 0;
			if (static_cast<int>(flags & BxDFType::Specular) && 
				static_cast<int>(flags & BxDFType::Transmission)) 
			{
				Float eta = isect.bsdf->eta;
				// Update the term that tracks radiance scaling for refraction
				// depending on whether the ray is entering or leaving the medium.
				// 放大还是缩小取决于光线是进入还是离开 medium
				etaScale *= (Dot(wo, isect.n) > 0) ? (eta * eta) : 1 / (eta * eta);
			}
			ray = isect.generate_ray(wi);

			// Account for subsurface scattering, if applicable
			/*
			if (isect.bssrdf && (flags & BSDF_TRANSMISSION)) 
			{
				// Importance sample the BSSRDF
				SurfaceInteraction pi;
				Spectrum S = isect.bssrdf->Sample_S(
					scene, sampler.Get1D(), sampler.Get2D(), arena, &pi, &pdf);
				DCHECK(!std::isinf(beta.y()));
				if (S.IsBlack() || pdf == 0) break;
				beta *= S / pdf;

				// Account for the direct subsurface scattering component
				L += beta * UniformSampleOneLight(pi, scene, arena, sampler, false,
					lightDistribution->Lookup(pi.p));

				// Account for the indirect subsurface scattering component
				Spectrum f = pi.bsdf->Sample_f(pi.wo, &wi, sampler.Get2D(), &pdf,
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
			Spectrum rrBeta = beta * etaScale;
			//3次反射后开始考虑终止路径
			if (rrBeta.max_value() < rrThreshold && bounces > 3)
			{
				Float q = std::max((Float).05, 1 - rrBeta.max_value());	//终止概率
				if (sampler.get_1D() < q) break;
				beta /= 1 - q;
				DCHECK(!std::isinf(beta.luminance()));
			}
		}
		return L;
	}

private:
	//const std::string lightSampleStrategy;
	int maxDepth;
	const Float rrThreshold;	// Russian Roulette Threshold
	std::unique_ptr<LightDistribution> lightDistribution;
};

}	//namespace valley


#endif //VALLEY_INTEGRATOR_PATH_H
