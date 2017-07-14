#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_SAMPLER_H
#define VALLEY_CORE_SAMPLER_H

#include"valley.h"
#include"rng.h"
#include"camera.h"

namespace valley
{

class Sampler
{
public:
	Sampler(int samples_PerPixel = 16, int seed = 1234) :
		samples_PerPixel(samples_PerPixel),rng(seed) {}

	Float   get()    { return rng.get(); }
	Point2f get_2D() { return rng.get_2D(); }
	Point3f get_3D() { return rng.get_3D(); }

	Float*   get_array() {}
	Point2f* get_2D_array() { }

	CameraSample get_CameraSample(int x, int y) 
	{
		CameraSample cs;
		cs.pFilm = Point2f(x, y) + rng.get_2D();
		return cs;
	}

public:
	RNG rng;
	int samples_PerPixel;
};

}	//namespace valley


#endif //VALLEY_CORE_SAMPLER_H

