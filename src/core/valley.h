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
class Color;
class RNG;
class Image;
class Scene;
class Integrator;
class SamplerIntegrator;
class Isect;
class SurfaceIsect;
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
class RNG;

struct Options 
{
    int nThreads = 0;
    bool quickRender = false;
    bool quiet = false;
    bool cat = false, toPly = false;
    std::string imageFile;
};

}	//namespace valley

#endif //VALLEY_CORE_VALLEY_H

