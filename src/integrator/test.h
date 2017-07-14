#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_INTEGRATOR_TEST_H
#define VALLEY_INTEGRATOR_TEST_H

#include"integrator.h"

namespace valley
{

class Test : public SamplerIntegrator
{
public:
	Test(std::shared_ptr<Camera> camera, std::shared_ptr<Sampler> sampler) :
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

	void render(const Scene &scene)
	{
		preprocess(scene, *sampler);

		for (int y = 0; y < camera->film->height; ++y)
			for (int x = 0; x < camera->film->width; ++x)
			{
				for (int count = 0; count < sampler->samples_PerPixel; ++count)
				{
					camera->film->add(camera->film->width*(sampler->get()), 
						camera->film->height*(sampler->get()), Color(0.1f));
				}
			}

		//对一个像素采样n次，可否理解为长时间曝光
		return camera->film->scale(1.f / sampler->samples_PerPixel);	//filter、flush
	}
};

}	//namespace valley


#endif //VALLEY_INTEGRATOR_TEST_H