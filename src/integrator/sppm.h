#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_INTEGRATOR_SPPM_H
#define RAINY_INTEGRATOR_SPPM_H

#include"rainy.h"
#include"integrator.h"

namespace rainy
{

class SPPM : public Integrator
{
public:
	SPPM(const Scene &scene, std::shared_ptr<Camera> camera, std::shared_ptr<Sampler> sampler,
		int nIterations = 64, int photons_PerIteration = 1024, int maxDepth = 5,
		Float initial_SearchRadius = 1.f, int writeFrequency = 8) :
		Integrator(camera, sampler), initial_SearchRadius(initial_SearchRadius),
		nIterations(nIterations), maxDepth(maxDepth),
		photons_PerIteration(photons_PerIteration > 0
			? photons_PerIteration
			: camera->film->bounds.Area()),
		writeFrequency(writeFrequency) {}
	~SPPM() {}

	void render(const Scene& scene);
	void interactive(const Scene& scene, int x, int y){}

private:
	const Float initial_SearchRadius;
	const int nIterations;
	const int maxDepth;
	const int photons_PerIteration;
	const int writeFrequency;
};

}	//namespace rainy


#endif //RAINY_INTEGRATOR_SPPM_H
