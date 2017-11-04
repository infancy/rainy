#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_RNG_H
#define RAINY_CORE_RNG_H

#include <random>

#include "geometry.h"
#include "rainy.h"

namespace rainy
{

static const double DoubleOneMinusEpsilon = 0.99999999999999989;
static const float FloatOneMinusEpsilon = 0.99999994;

#ifdef RAINY_FLOAT_AS_DOUBLE
static const Float OneMinusEpsilon = DoubleOneMinusEpsilon;
#else
static const Float OneMinusEpsilon = FloatOneMinusEpsilon;
#endif

//Random Number Generator

class RNG
{
public:
	RNG(int seed = 1234) : mRng(seed) {}

	int      get_int()  { return int_distr(mRng); }	    //(0, 2^21)
	uint32_t get_uint() { return uint_distr(mRng); }	//(0, 2^32)

	Float    get_1D() { return Float_distr(mRng); }	//(0.F, 1.F)
	Point2f  get_2D() { return Point2f(get_1D(), get_1D()); }
	//Point3f get_3D() { return Point3f(get(), get(), get()); }

private:
	std::mt19937_64 mRng;
	std::uniform_int_distribution<int>        int_distr;
	std::uniform_int_distribution<uint32_t>   uint_distr;
	std::uniform_real_distribution<Float>     Float_distr;
};

}	//namespace rainy


#endif //RAINY_CORE_RNG_H
