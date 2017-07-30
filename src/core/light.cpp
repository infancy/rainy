#include"light.h"
#include"interaction.h"
#include"spectrum.h"
#include"scene.h"

namespace valley
{

// Light Method Definitions
Light::Light(int flags, const Transform &LightToWorld,
//	const MediumInterface &mediumInterface, 
	int nSamples)
	: flags(flags),
	nSamples(std::max(1, nSamples)),
//	mediumInterface(mediumInterface),
	LightToWorld(LightToWorld),
	WorldToLight(Inverse(LightToWorld)) {}

Light::~Light() {}

bool Visibility::unoccluded(const Scene &scene) const 
{
	return !scene.intersectP(p0.generate_ray(p1));
}

Spectrum Visibility::Tr(const Scene &scene, Sampler &sampler) const
{
	Ray ray(p0.generate_ray(p1));
	Spectrum Tr(1.f);
	while (true) 
	{
		SurfaceInteraction isect;
		bool hitSurface = scene.intersect(ray, &isect);
		// Handle opaque surface along ray's path
		if (hitSurface && isect.primitive->get_material() != nullptr)
			return Spectrum(0.0f);

		// Update transmittance for current ray segment
		//if (ray.medium) Tr *= ray.medium->Tr(ray, sampler);

		// Generate next ray segment or return final transmittance
		if (!hitSurface) break;
		ray = isect.generate_ray(p1);
	}
	return Tr;
}

Spectrum Light::Le(const RayDifferential &ray) const { return Spectrum(0.f); }

AreaLight::AreaLight(const Transform& LightToWorld,
  //const MediumInterface& medium,
	int nSamples) : 
	Light((int)LightType::Area, LightToWorld, 
	//medium, 
	nSamples) {}

}	//namespace valley