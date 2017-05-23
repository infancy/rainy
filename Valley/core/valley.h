#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_VALLEY_H
#define VALLEY_CORE_VALLEY_H

#include"platform.h"

// Global Include Files
#include <type_traits>
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <cinttypes>
#include <cmath>
#include <cassert>
#include <cstring>
#ifdef VALLEY_HAVE_MALLOC_H
#include <malloc.h>  // for _alloca, memalign
#endif

#include <glog/logging.h>

// Platform-specific definitions
#include <stdint.h>
#if defined(VALLEY_IN_MSVC)
#include <float.h>
#include <intrin.h>
#pragma warning(disable : 4305)  // double constant assigned to float
#pragma warning(disable : 4244)  // int -> float conversion
#pragma warning(disable : 4843)  // double -> float conversion
#endif

namespace valley
{

//#define VALLEY_FLOAT_AS_DOUBLE
#ifdef VALLEY_FLOAT_AS_DOUBLE
	using Float = double;
#else
	using Float = float;
#endif  

//#define VALLEY_USE_EMBREE
#ifdef VALLEY_USE_EMBREE
	//
#endif

// Global Forward Declarations
class Scene;
class Integrator;
class SamplerIntegrator;
struct Interaction;
class SurfaceInteraction;
class Shape;
class Primitive;
class GeometricPrimitive;
template <int nSpectrumSamples>
class CoefficientSpectrum;
class RGBSpectrum;
class SampledSpectrum;
typedef RGBSpectrum Spectrum;
// typedef SampledSpectrum Spectrum;
class Camera;
struct CameraSample;
class ProjectiveCamera;
class Sampler;
class Filter;
class Film;
class FilmTile;
class BxDF;
class BRDF;
class BTDF;
class BSDF;
class Material;
template <typename T>
class Texture;
class Medium;
class MediumInteraction;
struct MediumInterface;
class BSSRDF;
class SeparableBSSRDF;
class TabulatedBSSRDF;
struct BSSRDFTable;
class Light;
class VisibilityTester;
class AreaLight;
struct Distribution1D;
class Distribution2D;
class RNG;
class MemoryArena;
template <typename T, int logBlockSize = 2>
class BlockedArray;
class ParamSet;
template <typename T>
struct ParamSetItem;
struct Options {
    int nThreads = 0;
    bool quickRender = false;
    bool quiet = false;
    bool cat = false, toPly = false;
    std::string imageFile;
};

extern Options PbrtOptions;
class TextureParams;

}	//namespace valley

#endif //VALLEY_CORE_VALLEY_H

