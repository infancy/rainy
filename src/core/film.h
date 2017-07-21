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
#include"filter.h"

namespace valley
{

struct Pixel
{
	Pixel() { FilterWeightSum = 0.0; }
	Color c;
	Float FilterWeightSum;
};

/*
(0,0)_ _ _ _ _ _x 方向
	|
	|
	|	film 的坐标系
	|	
	|
  y 方向
*/

class Film	
{
public:
	Film(int width = 800, int height = 600, Float resolution = 1.f, Filter* filter = nullptr, 
		 const std::string& filename = std::string("C:\\Users\\wyh32\\Desktop\\valley\\"),
		 bool save_type = false) 
		: width(width), height(height), 
		bounds(Point2i(0, 0), Point2i(width, height)),
		resolution(resolution), 
		pixels(new Pixel[width * height]),
		filter(filter), FilterRadius(filter->radius), 
		invFilterRadius(1. / FilterRadius.x, 1. / FilterRadius.y),
		filename(filename), 
		save_type(save_type) 
	{
		// Precompute filter weight table
		int offset = 0;
		for (int y = 0; y < kernelWidth; ++y)
			for (int x = 0; x < kernelWidth; ++x, ++offset)
			{
				Point2f p;
				p.x = (x + 0.5f) * filter->radius.x / kernelWidth;
				p.y = (y + 0.5f) * filter->radius.y / kernelWidth;
				kernel[offset] = filter->evaluate(p);
			}
	}

	~Film() {}

	Pixel& operator()(int y, int x)
	{
		DCHECK(0 <= x && x < width && 0 <= y && y < height);
		return pixels[y * width + x];
	}
	Pixel operator()(int y, int x) const
	{
		DCHECK(0 <= x && x < width && 0 <= y && y < height);
		return pixels[y * width + x];
	}

	void scale(Float factor)
	{
		for (int i = 0; i < width * height; ++i)
			pixels[i].c *= factor;
	}

	void add(const Point2f& pFilm, Color& L, Float sampleWeight = 1.0)
	{
		// Compute sample's raster bounds
		Point2f pFilmDiscrete = pFilm - Vector2f(0.5f, 0.5f);
		Point2i p0 = (Point2i)Ceil(pFilmDiscrete - filter->radius);
		Point2i p1 = (Point2i)Floor(pFilmDiscrete + filter->radius) + Point2i(1, 1);
		//以离散的 pFilmDiscrete 为中心，计算滤波的范围
		p0 = Max(p0, bounds.pMin);
		p1 = Min(p1, bounds.pMax);

		// Loop over filter support and add sample to pixel arrays

		// Precompute $x$ and $y$ filter table offsets
		//预先计算所有的偏移量
		std::vector<int> ifx(p1.x - p0.x);
		for (int x = p0.x; x < p1.x; ++x)
		{
			Float fx = std::abs((x - pFilmDiscrete.x) * invFilterRadius.x *
				kernelWidth);
			ifx[x - p0.x] = std::min((int)std::floor(fx), kernelWidth - 1);
		}

		std::vector<int> ify(p1.y - p0.y);
		for (int y = p0.y; y < p1.y; ++y)
		{
			Float fy = std::abs((y - pFilmDiscrete.y) * invFilterRadius.y *
				kernelWidth);
			ify[y - p0.y] = std::min((int)std::floor(fy), kernelWidth - 1);
		}

		for (int y = p0.y; y < p1.y; ++y)
			for (int x = p0.x; x < p1.x; ++x) 
			{
				// Evaluate filter value at $(x,y)$ pixel
				int offset = ify[y - p0.y] * kernelWidth + ifx[x - p0.x];

				Float filterWeight = kernel[offset];

				// Update pixel values with filtered sample contribution
				//根据像素滤波方程分别计算分子和分母，在最后保存图像的时候进行除法操作
				Pixel& pixel = (*this)(y, x);
				pixel.c += L * sampleWeight * filterWeight;    //这里是 += 号
				pixel.FilterWeightSum += filterWeight;
			}
	}

	void flush()
	{
		if (!filename.empty())
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

		//执行像素滤波的最后一步除法操作
		std::unique_ptr<Color[]> colors(new Color[width * height]);

		for(int i = 0; i < width * height; ++i)
		{ 
			Pixel& p = pixels[i];
			Color& c = colors[i];

			if (p.FilterWeightSum != 0)
				c = p.c / p.FilterWeightSum;
		}

		save_ppm(filename, colors.get(), width, height);
	}

public:
	int width, height;
	Bounds2i bounds;
	Float resolution;	//分辨率为单位面积的上的像素数量

private:
	std::unique_ptr<Pixel[]> pixels;

	std::unique_ptr<Filter> filter;
	Vector2f FilterRadius, invFilterRadius;
	static const int kernelWidth = 16;
	Float kernel[kernelWidth * kernelWidth];		//卷积核

	std::string filename;
	bool save_type;
};


}	//namespace valley


#endif //VALLEY_CORE_FILM_H
