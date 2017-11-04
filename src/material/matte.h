#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_MATERIAL_MATTE_H
#define RAINY_MATERIAL_MATTE_H

#include"rainy.h"
#include"material.h"

namespace rainy
{

//哑光材质
class Matte : public Material
{
public:
	// MatteMaterial Public Methods
	Matte(const std::shared_ptr<Texture<Spectrum>>& kd,
		  const std::shared_ptr<Texture<Float>>& sigma,   //粗糙度
		  const std::shared_ptr<Texture<Float>>& bumpMap = nullptr);

	void compute_scattering(SurfaceInteraction* si, TransportMode mode,
		bool allowMultipleLobes) const override;

private:
	std::shared_ptr<Texture<Spectrum>> kd;
	std::shared_ptr<Texture<Float>> sigma;	//粗糙度
	std::shared_ptr<Texture<Float>> bumpMap;
};

}	//namespace rainy


#endif //RAINY_MATERIAL_MATTE_H
