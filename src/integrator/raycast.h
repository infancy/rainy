#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_INTEGRATOR_RAYCAST_H
#define VALLEY_INTEGRATOR_RAYCAST_H

#include"valley.h"
#include"integrator.h"

namespace valley
{

class RayCast : public SamplerIntegrator
{
public:
	RayCast(std::shared_ptr<Camera> camera, std::shared_ptr<Sampler> sampler, uint32_t maxDepth = 2,
			bool sampler_all_light = true) :
		SamplerIntegrator(camera, sampler, maxDepth), sampler_all_light(sampler_all_light) {}

	virtual void preprocess(const Scene& scene, Sampler& sampler) 
	{
		if (sampler_all_light) 
		{
			// Compute number of samples to use for each light
			for (const auto &light : scene.lights)
				nLightSamples.push_back(light->nSamples);

			// Request samples for sampling all lights
			//for (int i = 0; i < maxDepth; ++i) 
			//	for (size_t j = 0; j < scene.lights.size(); ++j) 
			//		sampler.Request2DArray(nLightSamples[j]);
			//		sampler.Request2DArray(nLightSamples[j]);							
		}
	}

	virtual Color Li(const Ray& ray, const Scene &scene,
		Sampler &sampler, int depth = 0) const
	{
		Color L(0.f);
		// Find closest ray intersection or return background radiance
		SurfaceIsect isect;

		//如果未被遮挡，则计算光源到视点的直接辐射度
		if (!scene.intersect(ray, &isect)) 
		{
			for (const auto& light : scene.lights) L += light->Le(ray);
			return L;
		}

		//否则计算这个交点向视点方向发射的辐射度
		// Compute scattering functions for surface Isect
		isect.compute_scattering(ray);
		if (!isect.bsdf)
			return Li(isect.generate_ray(ray.d), scene, sampler, depth);
		Vector3f wo = isect.wo;

		// Compute emitted light if ray hit an area light source
		//交点上的区域光源
		L += isect.Le(wo);
		//scene中的光源
		if (scene.lights.size() > 0) 
		{
			// Compute direct lighting for _DirectLightingIntegrator_ integrator
			if (sampler_all_light)
				L += uniform_sample_all_lights(isect, scene, sampler, nLightSamples);
			else
				L += uniform_sample_one_light(isect, scene, sampler);
		}

		if (depth + 1 < maxDepth)
		{
			Vector3f wi;
			// Trace rays for specular reflection and refraction
			L += specular_reflect(ray, isect, scene, sampler, depth);
			L += specular_transmit(ray, isect, scene, sampler, depth);
		}
		return L;
	}

private:
	bool sampler_all_light;
	std::vector<int> nLightSamples;	//记录对每个light的采样数量
};

}	//namespace valley


#endif //VALLEY_INTEGRATOR_RAYCAST_H