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

namespace valley
{


//#define PBRT_FLOAT_AS_DOUBLE
#ifdef PBRT_FLOAT_AS_DOUBLE
	using Float = double;
#else
	using Float = float;
#endif  

//#define EMBREE
#ifdef EMBREE
	//
#endif



}



#endif //VALLEY_CORE_VALLEY_H

