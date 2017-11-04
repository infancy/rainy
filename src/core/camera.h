#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_CAMERA_H
#define RAINY_CORE_CAMERA_H

#include"rainy.h"
#include"geometry.h"
#include"transform.h"
#include"film.h"

namespace rainy
{

struct CameraSample	//生成光线所需的采样值
{
	Point2f pFilm;
	Point2f pLens;
};


class Camera
{
public:
	Camera(Film* film = nullptr) : film(film) {}
	virtual ~Camera() {}

	//根据采样数据生成世界空间光线，并返回一个在最终生成图像上的权值
	virtual Float generate_ray(const CameraSample& sample, Ray* ray) const = 0;

	virtual Spectrum We(const Ray& ray, Point2f* pRaster2 = nullptr) const = 0;
	virtual void pdf_We(const Ray& ray, Float* pdfPos, Float* pdfDir) const = 0;
	virtual Spectrum sample_Wi(const Interaction& ref, const Point2f& u,
		Vector3f* wi, Float* pdf, Point2f* pRaster, Visibility* vis) const = 0;

public:
	std::shared_ptr<Film> film;
};

}	//namespace rainy


#endif //RAINY_CORE_CAMERA_H
