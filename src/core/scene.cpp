#include"scene.h"
#include"geometry.h"
#include"color.h"
#include"intersection.h"

namespace valley
{

// Scene Method Definitions
bool Scene::intersect(const Ray& ray, SurfaceIsect* isect) const
{
	DCHECK_NE(ray.d, Vector3f(0, 0, 0));
	return aggregate->intersect(ray, isect);
}

bool Scene::intersectP(const Ray &ray) const 
{
	DCHECK_NE(ray.d, Vector3f(0, 0, 0));
	return aggregate->intersectP(ray);
}

bool Scene::intersectTr(Ray ray, Sampler& sampler, SurfaceIsect* isect, Color* Tr) const 
{
	*Tr = Color(1.f);
	while (true) {
		bool hitSurface = intersect(ray, isect);
		// Accumulate beam transmittance for ray segment
		//if (ray.medium) *Tr *= ray.medium->Tr(ray, sampler);

		// Initialize next ray segment or terminate transmittance computation
		if (!hitSurface) return false;
		if (isect->primitive->get_material() != nullptr) return true;
		ray = isect->generate_ray(ray.d);
		// return true; //Ä¬ÈÏ·µ»Øtrue
	}
}

}	//namespace valley