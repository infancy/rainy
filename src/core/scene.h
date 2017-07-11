#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_SCENE_H
#define VALLEY_CORE_SCENE_H

#include"valley.h"
#include"primitive.h"
#include"light.h"

namespace valley
{

class Scene 
{
public:
	Scene(Primitive* accelerator,
		  const std::vector<std::shared_ptr<Light>>& lights)
		: lights(lights), accelerator(accelerator) 
	{
		// Scene Constructor Implementation
		worldBound = accelerator->world_bound();
		for (const auto& light : lights)
		{
			light->preprocess(*this);
			if (light->flags & static_cast<int>(Light_type::Infinite))
				infiniteLights.push_back(light);
		}
	}
	const Bounds3f &WorldBound() const { return worldBound; }

	bool intersect(const Ray &ray, SurfaceIsect *isect) const;

	bool intersectP(const Ray &ray) const;
	//transmittance
	bool intersectTr(Ray ray, Sampler &sampler, SurfaceIsect *isect,
		Color *transmittance) const;

	std::vector<std::shared_ptr<Light>> lights;
	// Store infinite light sources separately for cases where we only want
	// to loop over them.
	std::vector<std::shared_ptr<Light>> infiniteLights;

private:
	std::shared_ptr<Primitive> accelerator;
	Bounds3f worldBound;
};

}	//namespace valley


#endif //VALLEY_CORE_SCENE_H
