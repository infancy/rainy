#include"scene.h"
#include"geometry.h"
#include"spectrum.h"
#include"interaction.h"

namespace rainy
{

// Scene Method Definitions
bool Scene::intersect(const Ray& ray, SurfaceInteraction* isect) const
{
	DCHECK_NE(ray.d, Vector3f(0, 0, 0));
	return accelerator->intersect(ray, isect);
}

bool Scene::intersectP(const Ray &ray) const 
{
	DCHECK_NE(ray.d, Vector3f(0, 0, 0));
	return accelerator->intersectP(ray);
}

bool Scene::intersectTr(Ray ray, Sampler& sampler, SurfaceInteraction* isect, Spectrum* Tr) const 
{
	*Tr = Spectrum(1.f);
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

}	//namespace rainy
