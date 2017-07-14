#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_INTEGRATOR_EYELIGHT_H
#define VALLEY_INTEGRATOR_EYELIGHT_H

#include"integrator.h"

namespace valley
{

class EyeLight : public SamplerIntegrator
{
public:
	EyeLight(std::shared_ptr<Camera> camera, std::shared_ptr<Sampler> sampler) :
		SamplerIntegrator(camera, sampler, 1) {}

	virtual Color Li(const Ray& ray, const Scene &scene,
		Sampler &sampler, int depth = 0) const 
	{
		SurfaceIsect isect;

		//cout << "x: " << x << " y: " << y << "\n";
		if (scene.intersect(ray, &isect))
		{
			Float dotLN = Dot(isect.n, -ray.d);

			if (dotLN > 0.f)
				return Color(dotLN);
			else
				return Color(-dotLN, 0, 0);
		}
		else
			return Color();
	}
};

}	//namespace valley


#endif //VALLEY_INTEGRATOR_EYELIGHT_H
