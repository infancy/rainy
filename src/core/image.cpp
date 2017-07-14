#include"image.h"
#include"color.h"
#include<fstream>

namespace valley
{

void save_ppm(std::string filename, const Float* pixel,
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

			if (r < 0 || r > 255) r = 1;
			if (g < 0 || g > 255) g = 1;
			if (b < 0 || b > 255) b = 1;

			ppm << r << " " << g << " " << b << " ";
		}

		ppm << std::endl;
	}
}

void save_ppm(std::string filename, const Color* pixel,
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

			if (r < 0 || r > 255) r = 1;
			if (g < 0 || g > 255) g = 1;
			if (b < 0 || b > 255) b = 1;

			ppm << r << " " << g << " " << b << " ";
		}

		ppm << std::endl;
	}
}

}	//namespace valley