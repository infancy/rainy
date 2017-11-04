#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_SAMPLER_RANDOM_H
#define RAINY_SAMPLER_RANDOM_H

#include"rainy.h"
#include"sampler.h"

namespace rainy
{

class RandomSampler : public Sampler
{
public:
	RandomSampler(int64_t samplesPerPixel = 16, int seed = 1234) :
		Sampler(samplesPerPixel), rng(seed) {}
	~RandomSampler() {}

	Float   get_1D() { return rng.get_1D(); }
	Point2f get_2D() { return rng.get_2D(); }

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
		return std::make_unique<RandomSampler>(samples_PerPixel, seed);
	}

public:
	RNG rng;
};

}	//namespace rainy


#endif //RAINY_SAMPLER_RANDOM_H
