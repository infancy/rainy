#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_PLATFORM_H
#define VALLEY_CORE_PLATFORM_H

#if defined(_WIN32) || defined(_WIN64)
	#define VALLEY_IN_WINDOWS
	#if defined(_MSC_VER)
		#define VALLEY_IN_MSVC
	#elif defined(__MINGW32__)  
		#define VALLEY_IN_MINGW
	#endif
#elif defined(__linux__)
	#define VALLEY_IN_LINUX
#elif defined(__APPLE__)
	#define VALLEY_IN_OSX
#endif

/*
#if defined(_MSC_VER) 
	#if _MSC_VER == 1910
	#define VALLEY_IN_MSVC2017
	#elif _MSC_VER == 1900
	#define VALLEY_IN_MSVC2015
	#endif
#endif
*/

//platform features

#if defined(VALLEY_IN_LINUX) || defined(VALLEY_IN_WINDOWS)
#define VALLEY_HAVE_MALLOC_H
#endif

#if defined(VALLEY_IN_MSVC)
#define VALLEY_THREAD_LOCAL __declspec(thread)
#else
#define VALLEY_THREAD_LOCAL __thread
#endif

#if defined(VALLEY_IN_MSVC)
#define VALLEY_FORCEINLINE __forceinline
#else
#define VALLEY_FORCEINLINE __attribute__((always_inline)) inline
#endif

#endif //VALLEY_CORE_PLATFORM_H



