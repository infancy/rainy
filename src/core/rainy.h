#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_RAINY_H
#define RAINY_CORE_RAINY_H

#include"platform.h"

// Global Include Files
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>
#ifdef RAINY_HAVE_MALLOC_H
#include <malloc.h>  // for _alloca, memalign
#endif

#include <glog/logging.h>

// Platform-specific definitions
#include <stdint.h>
#if defined(RAINY_IN_MSVC)
#include <float.h>
#include <intrin.h>
#pragma warning(disable : 4267)  // size_t -> int conversion
#pragma warning(disable : 4305)  // double constant assigned to float
#pragma warning(disable : 4244)  // int -> float conversion
#pragma warning(disable : 4843)  // double -> float conversion
#endif

namespace rainy
{

//#define RAINY_FLOAT_AS_DOUBLE
#ifdef RAINY_FLOAT_AS_DOUBLE
	using Float = double;
#else
	using Float = float;
#endif  

//#define RAINY_USE_EMBREE
#ifdef RAINY_USE_EMBREE
	//
#endif

//the math module of pbrt
template <typename T>
class Vector2;
template <typename T>
class Vector3;
template <typename T>
class Point2;
template <typename T>
class Point3;
template <typename T>
class Normal3;
class Ray;
class RayDifferential;
template <typename T>
class Bounds2;
template <typename T>
class Bounds3;
class Transform;
struct Matrix4x4;

// Global Forward Declarations
class Spectrum;
class RNG;
class Image;
class Scene;
class Integrator;
class SamplerIntegrator;
class Interaction;
class SurfaceInteraction;
class MediumInteraction;
class Shape;
class Primitive;
class GeometricPrimitive;

class Camera;
struct CameraSample;
class ProjectiveCamera;
class Sampler;
class Filter;
class Film;
class FilmTile;
class BxDF;
class BSDF;
//class BSSRDF;
class Material;
template <typename T>
class Texture;
class Medium;

class Light;
class Visibility;
class AreaLight;
struct Distribution1D;
class Distribution2D;

//用于确定路径是从相机还是光源开始的
enum class TransportMode { Radiance, Importance };

struct Options 
{
    int nThreads = 0;
    bool quickRender = false;
    bool quiet = false;
    bool cat = false, toPly = false;
    std::string imageFile;
};

}	//namespace rainy

#endif //RAINY_CORE_RAINY_H

