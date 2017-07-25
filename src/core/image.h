#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_IMAGE_H
#define VALLEY_CORE_IMAGE_H

#include "valley.h"

namespace valley
{

void save_ppm(std::string filename, const Float* pixel,
			 int width, int height);

void save_ppm(std::string filename, const Spectrum* pixel,
	int width, int height, bool red = false);

}	//namespace valley


#endif //VALLEY_CORE_IMAGE_H
