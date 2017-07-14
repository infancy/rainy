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
	std::shared_ptr<Film> film;
};


//class ProjectiveCamera : public Camera


class Pinhole : public Camera	//PerspectiveCamera
{
public:
	Pinhole(const Point3f eye, const Point3f target, const Vector3f up,
		   Float imageDistance, Film* film)
		: Camera(film), eye(eye), imageDistance(imageDistance)
	{
		forward = Normalize(target - eye);
		CHECK(!(up.x == forward.x && up.z == forward.z));		
		right   = Normalize(Cross(up, forward));			//左手叉乘
		upward  = Cross(forward, right);

		Matrix4x4 c2w(right.x, upward.x, forward.x, eye.x,
					  right.y, upward.y, forward.y, eye.y,
					  right.z, upward.z, forward.z, eye.z,
					0, 0, 0, 1);
		camera_to_world = c2w;

		//车祸现场
		//Float pixelSize = 1.f / std::sqrt(film->resolution);
		Float a = std::sqrt(film->resolution);
		//Float pixelSize = 1.f / a;
		pixelSize = 1.f / a;

		//raster_to_camera
	}

	Float generate_ray(const CameraSample& sample, Ray* ray) const
	{
		Point3f pFilm(sample.pFilm.x, sample.pFilm.y, 0);	
		Point3f p(pixelSize * (pFilm.x - film->width / 2),		//dir - (0,0,0)
					 pixelSize * (film->height / 2 - pFilm.y),
					 imageDistance);

		//Vector3f dir = forward * pCamera.z + right * pCamera.x + upward * pCamera.y;
		p = camera_to_world(p);
		*ray = Ray(eye, Normalize(p - eye));

		return 1;
	}

private:
	Point3f eye;
	Vector3f forward, right, upward;

	Transform camera_to_world;

	Float imageDistance, pixelSize;	//图像平面到针孔的距离、像素实际尺寸
};

}	//namespace valley


#endif //VALLEY_CORE_CAMERA_H
