#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_IMAGE_H
#define VALLEY_CORE_IMAGE_H

#include "valley.h"

namespace valley
{

void save_ppm(const char* filename, const Float* pixel,
			 int width, int height);

void save_ppm(const char* filename, const Color4f* pixel,
	int width, int height);

}	//namespace valley


#endif //VALLEY_CORE_IMAGE_H
