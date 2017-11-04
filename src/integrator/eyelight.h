#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_INTEGRATOR_EYELIGHT_H
#define RAINY_INTEGRATOR_EYELIGHT_H

#include"integrator.h"

namespace rainy
{

class EyeLight : public SamplerIntegrator
{
public:
	EyeLight(const Scene& scene, std::shared_ptr<Camera> camera, std::shared_ptr<Sampler> sampler) :
		SamplerIntegrator(camera, sampler) {}

	virtual Spectrum Li(const Ray& ray, const Scene &scene,
		Sampler &sampler, int depth = 0) const 
	{
		SurfaceInteraction isect;

		//cout << "x: " << x << " y: " << y << "\n";
		if (scene.intersect(ray, &isect))
		{
			Float dotLN = Dot(isect.n, -ray.d);

			if (dotLN > 0.f)
				return Spectrum(dotLN);
			else
				return Spectrum(-dotLN, 0, 0);
		}
		else
			return Spectrum();
	}
};

}	//namespace rainy


#endif //RAINY_INTEGRATOR_EYELIGHT_H
