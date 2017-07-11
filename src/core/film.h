#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_FILM_H
#define VALLEY_CORE_FILM_H

#include"valley.h"
#include"color.h"
#include"geometry.h"
#include"image.h"

namespace valley
{

class Film	//: public Image
{
public:
	Film(int width = 800, int height = 600, Float resolution = 1.f, 
		const std::string& filename = std::string()) 
	    : width(width), height(height), resolution(resolution), 
		filename("C:\\Users\\wyh32\\Desktop\\valley\\"),
		pixels(new Color[width * height]) {}
	~Film() 
	{ 
		if(!filename.empty())
		{ 
			std::string time;
			long t = clock();
			while (t != 0)
			{
				char c = '0';
				time += (c + t % 10);
				t /= 10;
			}
			std::reverse(time.begin(), time.end());
			filename += time;
			filename += ".ppm";
		}

		save_ppm(filename, pixels.get(), width, height);
	}

	Color& operator()(int x, int y)
	{
		DCHECK(0 <= x && x < width && 0 <= y && y < height);
		return pixels[y * width + x];
	}
	Color operator()(int x, int y) const
	{
		DCHECK(0 <= x && x < width && 0 <= y && y < height);
		return pixels[y * width + x];
	}

public:
	int width, height;
	Float resolution;	//分辨率为单位面积的上的像素数量

private:
	std::unique_ptr<Color[]> pixels;
	std::string filename;
};


}	//namespace valley


#endif //VALLEY_CORE_FILM_H
