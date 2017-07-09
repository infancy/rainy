#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_SCENE_H
#define VALLEY_CORE_SCENE_H

#include"valley.h"

namespace valley
{

class Scene {
public:
	// Scene Public Methods
	Scene(std::shared_ptr<Primitive> aggregate,
		const std::vector<std::shared_ptr<Light>> &lights)
		: lights(lights), aggregate(aggregate) {
		// Scene Constructor Implementation
		worldBound = aggregate->WorldBound();
		for (const auto &light : lights) {
			light->Preprocess(*this);
			if (light->flags & static_cast<int>(Light_type::Infinite))
				infiniteLights.push_back(light);
		}
	}
	const Bounds3f &WorldBound() const { return worldBound; }
	bool Intersect(const Ray &ray, SurfaceIsect *isect) const;
	bool IntersectP(const Ray &ray) const;
	bool IntersectTr(Ray ray, Sampler &sampler, SurfaceIsect *isect,
		Color *transmittance) const;

	// Scene Public Data
	std::vector<std::shared_ptr<Light>> lights;
	// Store infinite light sources separately for cases where we only want
	// to loop over them.
	std::vector<std::shared_ptr<Light>> infiniteLights;

private:
	// Scene Private Data
	std::shared_ptr<Primitive> aggregate;
	Bounds3f worldBound;
};

}	//namespace valley


#endif //VALLEY_CORE_SCENE_H
