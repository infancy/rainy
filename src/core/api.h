#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_API_H
#define VALLEY_CORE_API_H

#include"camera.h"
#include"film.h"
#include"integrator.h"
#include"light.h"
#include"primitive.h"
#include"scene.h"
#include"texture.h"

#include"integrator/eyelight.h"
#include"integrator/raycast.h"
#include"integrator/test.h"
#include"light/diffuse.h"
#include"light/distance.h"
#include"light/point.h"
#include"material/matte.h"
#include"shape/sphere.h"
#include"shape/Rectangle.h"

using namespace std;

namespace valley
{

enum class Intetrator_type
{

};

void valley_render();


shared_ptr<Scene>      valley_create_scene();

Integrator* valley_create_integrator();

}	//namespace valley


#endif //VALLEY_CORE_API_H

