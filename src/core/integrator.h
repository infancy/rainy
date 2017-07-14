#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_INTEGTATOR_H
#define VALLEY_CORE_INTEGTATOR_H

#include"valley.h"
#include"color.h"
#include"geometry.h"
#include"intersection.h"
#include"lightdistrib.h"
#include"camera.h"
#include"sampler.h"
#include"scene.h"

namespace valley
{

//均匀采样多个光源的radiance
Color uniform_sample_all_lights(const Isect& it, const Scene &scene, Sampler& sampler,
								const std::vector<int>& nLightSamples, bool handleMedia = false);

//采样单个光源并除以其power_pdf，得到近似采样多个光源的结果
Color uniform_sample_one_light(const Isect& it, const Scene& scene, Sampler& sampler,
							   bool handleMedia = false, const Distribution1D* lightDistrib = nullptr);

//选定一个Isect和一个Light，使用双重重要性采样来计算直接光照的贡献
Color estimate_direct(const Isect& it,    const Point2f& uScattering,
					  const Light& light, const Point2f& uLight, 
					  const Scene& scene, Sampler& sampler,
					  bool handleMedia = false, bool has_specular = false);

class Integrator 
{
public:
	// Integrator Interface
	virtual ~Integrator() {}
	virtual void render(const Scene &scene) = 0;
};

class SamplerIntegrator : public Integrator
{
public:
	SamplerIntegrator(std::shared_ptr<Camera> camera,  std::shared_ptr<Sampler> sampler,
			   uint32_t maxDepth) : camera(camera), sampler(sampler), maxDepth(maxDepth) {}
	virtual ~SamplerIntegrator() {}

	virtual void preprocess(const Scene& scene, Sampler& sampler) {}
	virtual void render(const Scene& scene) override;

	//计算沿光线的辐射度
	virtual Color Li(const Ray& ray, const Scene &scene,
		Sampler &sampler, int depth = 0) const = 0;
	//	{std::cerr << "error,should't call Li() in Integrator_base_class\n";
	//   return Color(); }

	Color specular_reflect(const Ray& ray, const SurfaceIsect& isect,
						  const Scene &scene, Sampler &sampler, int depth) const;
	Color specular_transmit(const Ray& ray, const SurfaceIsect& isect,  
						   const Scene &scene, Sampler &sampler, int depth) const;

protected:
	uint32_t maxDepth;

	std::shared_ptr<Sampler> sampler;
	std::shared_ptr<Camera> camera;
};

}	//namespace valley


#endif //VALLEY_CORE_INTEGTATOR_H
