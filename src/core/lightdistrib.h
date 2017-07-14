#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_LIGHTDISTRIB_H
#define VALLEY_CORE_LIGHTDISTRIB_H

#include"valley.h"
#include"geometry.h"
#include"integrator.h"
#include"sampling.h"
#include"scene.h"

namespace valley
{

// LightDistribution defines a general interface for classes that provide
// probability distributions for sampling light sources at a given point in
// space.
class LightDistribution
{
public:
	virtual ~LightDistribution() {}

	// Given a point |p| in space, this method returns a (hopefully
	// effective) sampling distribution for light sources at that point.
	virtual const Distribution1D* lookup(const Point3f &p) const = 0;
};

// The simplest possible implementation of LightDistribution: this returns
// a uniform distribution over all light sources, ignoring the provided
// point. This approach works well for very simple scenes, but is quite
// ineffective for scenes with more than a handful of light sources. (This
// was the sampling method originally used for the PathIntegrator and the
// VolPathIntegrator in the printed book, though without the
// UniformLightDistribution class.)
class UniformDistribution : public LightDistribution 
{
public:
	UniformDistribution(const Scene &scene)
	{
		std::vector<Float> prob(scene.lights.size(), Float(1));
		distrib.reset(new Distribution1D(&prob[0], prob.size()));
	}
	const Distribution1D* lookup(const Point3f &p) const { return distrib.get(); }

private:
	std::unique_ptr<Distribution1D> distrib;
};

inline std::unique_ptr<Distribution1D> light_power_distribution(const Scene &scene)
{
	if (scene.lights.empty()) return nullptr;
	std::vector<Float> lightPower;
	for (const auto &light : scene.lights)
		lightPower.push_back(light->power().luminance());
	return std::unique_ptr<Distribution1D>(
		new Distribution1D(&lightPower[0], lightPower.size()));
}

// PowerLightDistribution returns a distribution with sampling probability
// proportional to the total emitted power for each light. (It also ignores
// the provided point |p|.)  This approach works well for scenes where
// there the most powerful lights are also the most important contributors
// to lighting in the scene, but doesn't do well if there are many lights
// and if different lights are relatively important in some areas of the
// scene and unimportant in others. (This was the default sampling method
// used for the BDPT integrator and MLT integrator in the printed book,
// though also without the PowerLightDistribution class.)
class PowerDistribution : public LightDistribution 
{
public:
	PowerDistribution(const Scene &scene) : distrib(light_power_distribution(scene)) {}
	const Distribution1D* lookup(const Point3f &p) const { return distrib.get(); }

private:
	std::unique_ptr<Distribution1D> distrib;
};

}	//namespace valley


#endif //VALLEY_CORE_LIGHTDISTRIB_H

