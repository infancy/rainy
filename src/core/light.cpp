#include"light.h"
#include"interaction.h"
#include"spectrum.h"
#include"scene.h"

namespace rainy
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
		// 沿着光线的路径处理不透明的表面
		if (hitSurface && isect.primitive->get_material() != nullptr)
			return Spectrum(0.0f);

		// Update transmittance for current ray segment
		//if (ray.medium) Tr *= ray.medium->Tr(ray, sampler);

		// Generate next ray segment or return final transmittance
		// 当从 p1 所处的位置生成的 ray 进行 scene.intersect(ray, &isect) 时 hit Surface 为 false，推出循环
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

}	//namespace rainy
