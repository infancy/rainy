#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_API_H
#define VALLEY_CORE_API_H

#include"camera.h"
#include"film.h"
#include"primitive.h"
#include"scene.h"
#include"texture.h"

#include"material/matte.h"
#include"shape/sphere.h"
#include"shape/Rectangle.h"

using namespace std;

namespace valley
{

Scene* valley_create_scene();

}	//namespace valley


#endif //VALLEY_CORE_API_H

