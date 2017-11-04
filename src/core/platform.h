#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_PLATFORM_H
#define RAINY_CORE_PLATFORM_H

#if defined(_WIN32) || defined(_WIN64)
	#define RAINY_IN_WINDOWS
	#if defined(_MSC_VER)
		#define RAINY_IN_MSVC
	#elif defined(__MINGW32__)  
		#define RAINY_IN_MINGW
	#endif
#elif defined(__linux__)
	#define RAINY_IN_LINUX
#elif defined(__APPLE__)
	#define RAINY_IN_OSX
#endif

/*
#if defined(_MSC_VER) 
	#if _MSC_VER == 1910
	#define RAINY_IN_MSVC2017
	#elif _MSC_VER == 1900
	#define RAINY_IN_MSVC2015
	#endif
#endif
*/

//platform features

#if defined(RAINY_IN_LINUX) || defined(RAINY_IN_WINDOWS)
#define RAINY_HAVE_MALLOC_H
#endif

#if defined(RAINY_IN_MSVC)
#define RAINY_THREAD_LOCAL __declspec(thread)
#else
#define RAINY_THREAD_LOCAL __thread
#endif

#if defined(RAINY_IN_MSVC)
#define RAINY_FORCEINLINE __forceinline
#else
#define RAINY_FORCEINLINE __attribute__((always_inline)) inline
#endif

#endif //RAINY_CORE_PLATFORM_H



