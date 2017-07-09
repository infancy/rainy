#include"image.h"
#include"color.h"
#include<fstream>

namespace valley
{

void save_ppm(const char* filename, const Float* pixel,
	int width, int height)
{
	std::ofstream ppm(filename);
	//encode by ASCII
	ppm << "P3\n" << width << " " << height << "\n255\n";
	/*
	std::ofstream ppm(filename, std::ios::out | std::ios::binary);
	ppm << "P6\n" << width << " " << height << "\n255\n";
	*/

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x< width; ++x)
		{
			int r = *pixel * 255;
			int g = *(++pixel) * 255;
			int b = *(++pixel) * 255;

			ppm << std::min(255, std::max(0, r)) << " "
				<< std::min(255, std::max(0, g)) << " "
				<< std::min(255, std::max(0, b)) << " ";
		}

		ppm << std::endl;
	}
}

void save_ppm(const char* filename, const Color* pixel,
	int width, int height)
{
	std::ofstream ppm(filename);
	ppm << "P3\n" << width << " " << height << "\n255\n";

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x< width; ++x, ++pixel)
		{
			int r = pixel->r * 255;
			int g = pixel->g * 255;
			int b = pixel->b * 255;

			ppm << std::min(255, std::max(0, r)) << " "
				<< std::min(255, std::max(0, g)) << " "
				<< std::min(255, std::max(0, b)) << " ";
		}

		ppm << std::endl;
	}
}

}	//namespace valley