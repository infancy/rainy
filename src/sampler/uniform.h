#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_SAMPLER_UNIFORM_H
#define RAINY_SAMPLER_UNIFORM_H

#include"rainy.h"
#include"sampler.h"

namespace rainy
{

class UniformSampler : public Sampler
{
public:
	UniformSampler(int64_t samplesPerPixel = 16) :
		Sampler(samplesPerPixel){}
	~UniformSampler() {}

	Float   get_1D() { return 0.5; }
	Point2f get_2D() { return Point2f(0.5, 0.5); }

	/*
	virtual void start_pixel(const Point2i& p)
	{
		for (size_t i = 0; i < sampleArray_1D.size(); ++i)
			for (size_t j = 0; j < sampleArray_1D[i].size(); ++j)
				sampleArray_1D[i][j] = rng.get();

		for (size_t i = 0; i < sampleArray_2D.size(); ++i)
			for (size_t j = 0; j < sampleArray_2D[i].size(); ++j)
				sampleArray_2D[i][j] = rng.get_2D();

		Sampler::start_pixel(p);
	}
	*/

	virtual std::unique_ptr<Sampler> clone(int seed)
	{
		return std::make_unique<UniformSampler>(samples_PerPixel);
	}
};

}	//namespace rainy


#endif //RAINY_SAMPLER_UNIFORM_H
