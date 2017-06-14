#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_CAMERA_H
#define VALLEY_CORE_CAMERA_H

#include"valley.h"
#include"geometry.h"
#include"transform.h"
#include"film.h"

namespace valley
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

public:
	Film* film;
};


//class ProjectiveCamera : public Camera


class Pinhole : public Camera	//PerspectiveCamera
{
public:
	Pinhole(const Point3f eye, const Point3f target, const Vector3f up,
		   Float imageDistance, Film* film)
		: Camera(film), eye(eye), imageDistance(imageDistance)
	{
		//生成camera_to_world矩阵
		forward = Normalize(target - eye);
		right   = Normalize(Cross(up, forward));	//左手叉乘
		upward  = Cross(forward, right);

		//车祸现场
		//Float pixelSize = 1.f / std::sqrt(film->resolution);
		Float a = std::sqrt(film->resolution);
		//Float pixelSize = 1.f / a;
		pixelSize = 1.f / a;
	}

	Float generate_ray(const CameraSample& sample, Ray* ray) const
	{
		Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);	
		Point3f pCamera(pixelSize * (pFilm.x - film->width / 2),
						pixelSize * (film->height / 2 - pFilm.y),
						imageDistance);
		//Vec3 dir = p.x * u + p.y * v + vpd * w;
		Vector3f dir = forward * pCamera.z + right * pCamera.x + upward * pCamera.y;
		*ray = Ray(eye, Normalize(dir));

		//*ray = camera_to_world(*ray);
		return 1;
	}

private:
	Point3f eye;
	Vector3f forward, right, upward;

	Float imageDistance, pixelSize;	//图像平面到针孔的距离、像素实际尺寸
};

}	//namespace valley


#endif //VALLEY_CORE_CAMERA_H
