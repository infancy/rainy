#include"image.h"
#include"spectrum.h"
#include<fstream>

namespace rainy
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

			if (r < 0 || r > 255) r = 255;
			if (g < 0 || g > 255) g = 0;
			if (b < 0 || b > 255) b = 0;

			ppm << r << " " << g << " " << b << " ";
		}

		ppm << std::endl;
	}
}

void save_ppm(std::string filename, const Spectrum* pixel,
	int width, int height, bool red)
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

			if(red)
			{ 
				if (r < 0 || r > 255) r = 255;
				if (g < 0 || g > 255) g = 0;
				if (b < 0 || b > 255) b = 0;

				ppm << r << " " << g << " " << b << " ";
			}
			else
			{
				ppm << std::min(255, std::max(0, r)) << " "
					<< std::min(255, std::max(0, g)) << " "
					<< std::min(255, std::max(0, b)) << " ";
			}
		}

		ppm << std::endl;
	}
}

}	//namespace rainy
