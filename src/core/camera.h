#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_CAMERA_H
#define VALLEY_CORE_CAMERA_H

#include "valley.h"
#include "geometry.h"
#include "transform.h"
//#include "film.h"

namespace valley
{

struct CameraSample	//the ²ÉÑùµã of film plane and/or lens plane
{
	Point2f pFilm;
	Point2f pLens;
};

class Camera
{

};

}	//namespace valley


#endif //VALLEY_CORE_STENCIL_H
