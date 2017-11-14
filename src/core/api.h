#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_API_H
#define RAINY_CORE_API_H

#include "camera.h"
#include "film.h"
#include "filter.h"
#include "integrator.h"
#include "light.h"
#include "primitive.h"
#include "scene.h"
#include "texture.h"

#include "camera/perspective.h"

// #include "integrator/bdpt.h"
#include "integrator/eyelight.h"
#include "integrator/pathtracing.h"
#include "integrator/directlight.h"
#include "integrator/sppm.h"
// #include "integrator/test.h"

#include "light/diffuse.h"
#include "light/distance.h"
#include "light/point.h"

#include "material/glass.h"
#include "material/matte.h"
#include "material/mirror.h"

#include "sampler/random.h"
#include "sampler/uniform.h"

#include "shape/sphere.h"
#include "shape/rectangle.h"

namespace rainy
{

enum class Intetrator_type
{

};

void rainy_render();

std::shared_ptr<Scene> rainy_create_scene();

Integrator* rainy_create_integrator(const Scene& scene);

}	// namespace rainy


#endif //RAINY_CORE_API_H

