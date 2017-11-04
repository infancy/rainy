#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_SCENE_H
#define RAINY_CORE_SCENE_H

#include"rainy.h"
#include"primitive.h"
#include"light.h"

namespace rainy
{

class Scene 
{
public:
	Scene(Primitive* accelerator,
		  const std::vector<std::shared_ptr<Light>>& lights)
		: lights(lights), accelerator(accelerator) 
	{
		worldBound = accelerator->world_bound();
		//无穷光源等光源需要进行预处理
		for (const auto& light : lights)
		{
			light->preprocess(*this);
			if (light->flags & static_cast<int>(LightType::Infinite))
				infiniteLights.push_back(light);
		}
	}
	const Bounds3f& WorldBound() const { return worldBound; }

	bool intersect(const Ray& ray, SurfaceInteraction* isect) const;

	bool intersectP(const Ray& ray) const;
	//transmittance
	bool intersectTr(Ray ray, Sampler& sampler, SurfaceInteraction* isect,
		Spectrum* transmittance) const;

public:
	std::vector<std::shared_ptr<Light>> lights;
	// Store infinite light sources separately for cases where we only want
	// to loop over them.
	std::vector<std::shared_ptr<Light>> infiniteLights;

private:
	std::shared_ptr<Primitive> accelerator;
	Bounds3f worldBound;
};

}	//namespace rainy


#endif //RAINY_CORE_SCENE_H
