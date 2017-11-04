#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_IMAGE_H
#define RAINY_CORE_IMAGE_H

#include "rainy.h"

namespace rainy
{

void save_ppm(std::string filename, const Float* pixel,
			 int width, int height);

void save_ppm(std::string filename, const Spectrum* pixel,
	int width, int height, bool red = false);

}	//namespace rainy


#endif //RAINY_CORE_IMAGE_H
