#include"integrator.h"
#include"sampler.h"

namespace valley
{

// Integrator Utility Functions
//对所有光源进行uniform的采样
Spectrum uniform_sample_all_lights(const Interaction& it, const Scene &scene, Sampler &sampler,
	const std::vector<int> &nLightSamples, bool handleMedia)
{
	Spectrum L(0.f);
	for (size_t i = 0; i < scene.lights.size(); ++i) 
	{
		// Accumulate contribution of _j_th light to _L_
		const std::shared_ptr<Light>& light = scene.lights[i];
		int nSamples = nLightSamples[i];

		Spectrum Ld(0.f);
		for (int k = 0; k < nSamples; ++k)
			Ld += estimate_direct(it, sampler.get_2D(), *light, sampler.get_2D(), 
				scene, sampler, handleMedia);
		L += Ld / nSamples;	//在光源上取n个采样点，计算平均辐射度
	}
	return L;
}

Spectrum uniform_sample_one_light(const Interaction& it, const Scene& scene, Sampler& sampler,
	bool handleMedia, const Distribution1D* lightDistrib)
{
	// Randomly choose a single light to sample, _light_
	int nLights = int(scene.lights.size());
	if (nLights == 0) return Spectrum(0.f);
	int lightNum;
	Float lightPdf;

	if (lightDistrib)
	{
		//随机选取一个light
		lightNum = lightDistrib->sample_discrete(sampler.get_1D(), &lightPdf);
		if (lightPdf == 0) return Spectrum(0.f);
	}
	else
	{
		lightNum = std::min((int)(sampler.get_1D() * nLights), nLights - 1);
		lightPdf = Float(1) / nLights;
	}
	const std::shared_ptr<Light> &light = scene.lights[lightNum];
	//Point2f uLight = sampler.get_2D();
	//Point2f uScattering = sampler.get_2D();

	//最后除以lightPdf，得到一个放大的效果，近似于对所有灯光进行采样
	return estimate_direct(it, sampler.get_2D(), *light, sampler.get_2D(),
		scene, sampler, handleMedia) / lightPdf;
}

/*
   prev   light
    ----  ----
      \    /
    wo \  / wi
        \/
      ------
      isect
*/
//当bsdf呈镜面状态而光源分布较广时从bsdf采样较为高效
//当光源较小而bsdf呈漫反射分布时从光源采样更高效
//因而使用MIS分别对light和brdf进行采样
//在光源上采集一个点p计算Le，在bsdf上采集以方向wi计算Li，最后Ld=MIS(Le, Li)
//默认不计算镜面BSDF项
Spectrum estimate_direct(const Interaction& it, const Point2f &uScattering, const Light &light,
	const Point2f &uLight, const Scene &scene, Sampler &sampler,
	bool handleMedia, bool has_specular)
{
	// Sample light source with multiple importance sampling
	BxDF_type bsdfFlags = has_specular ? BxDF_type::All : BxDF_type::NonSpecular;
	Spectrum Ld(0.f);
	Vector3f wi;
	Float lightPdf = 0, scatteringPdf = 0;
	Visibility visibility;

	//计算Li,wi,pdf
	Spectrum Li = light.sample_Li(it, uLight, &wi, &lightPdf, &visibility);
	DVLOG(2) << "EstimateDirect uLight:" << uLight << " -> Li: " << Li << ", wi: "
		<< wi << ", pdf: " << lightPdf;

	//从光源部分进行采样
	if (lightPdf > 0 && !Li.is_black()) 
	{
		//分段函数
		// Compute BSDF or phase function's value for light sample
		Spectrum f;
		if (it.in_surface()) 
		{
			// Evaluate BSDF for light sampling strategy
			const SurfaceInteraction& isect = (const SurfaceInteraction&)it;
			//计算f(p,wo,wi)cos(eta_light_isect)项
			//f = isect.bsdf->f(isect.wo, wi, bsdfFlags) * AbsDot(wi, isect.shading.n);
			f = isect.bsdf->f(isect.wo, wi, bsdfFlags);
			f *= AbsDot(wi, isect.shading.n);
			scatteringPdf = isect.bsdf->pdf(isect.wo, wi, bsdfFlags);

			DVLOG(2) << "  surf f*dot :" << f << ", scatteringPdf: " << scatteringPdf;
		}
		/*
		else 
		{
			// Evaluate phase function for light sampling strategy
			const MediumInteraction &mi = (const MediumInteraction &)it;
			Float p = mi.phase->p(mi.wo, wi);
			f = Spectrum(p);
			scatteringPdf = p;
			VLOG(2) << "  medium p: " << p;
		}
		*/
		if (!f.is_black()) 
		{
			//可见性测试
			// Compute effect of visibility for light source sample
			if (handleMedia) 
			{
				Li *= visibility.Tr(scene, sampler);
				DVLOG(2) << "  after Tr, Li: " << Li;
			}
			else 
			{
				if (!visibility.unoccluded(scene))
				{
					DVLOG(2) << "  shadow ray blocked";
					Li = Spectrum(0.f);
				}
				else
					DVLOG(2) << "  shadow ray unoccluded";
			}

			// Add light's contribution to reflected radiance
			//计算最终结果
			if (!Li.is_black()) 
			{
				if (is_delta_light(light.flags))	//如果是delta光源，无需使用MIS
					Ld += f * Li / lightPdf;		//对普通光源，pdf=1，对区域光源，pdf=1/area
				else 
				{
					Float weight =
						power_heuristic(1, lightPdf, 1, scatteringPdf);
					Ld += f * Li * weight / lightPdf;
				}
			}
		}
	}

	//从bsdf部分进行采样
	if (!is_delta_light(light.flags)) 
	{
		Spectrum f;
		bool sampledSpecular = false;
		if (it.in_surface()) 
		{
			// Sample scattered direction for surface interactions
			BxDF_type sampledType;
			const SurfaceInteraction& isect = (const SurfaceInteraction&)it;
			//使用采样点对bsdf进行采样，得到wi，pdf，f
			f = isect.bsdf->sample_f(isect.wo, &wi, uScattering, &scatteringPdf,
				bsdfFlags, &sampledType);
			f *= AbsDot(wi, isect.shading.n);
			sampledSpecular = static_cast<int>(sampledType & BxDF_type::Specular) != 0;
		}
		/*
		else 
		{
			// Sample scattered direction for medium interactions
			const MediumInteraction &mi = (const MediumInteraction &)it;
			Float p = mi.phase->Sample_p(mi.wo, &wi, uScattering);
			f = Spectrum(p);
			scatteringPdf = p;
		}
		*/
		DVLOG(2) << "  BSDF / phase sampling f: " << f << ", scatteringPdf: " <<
			scatteringPdf;

		if (!f.is_black() && scatteringPdf > 0) 
		{
			// Account for light contributions along sampled direction _wi_
			//沿着wi的方向计算光源贡献
			Float weight = 1;
			if (!sampledSpecular) 
			{
				lightPdf = light.pdf_Li(it, wi);	//若it->wi->light,则采样到light的概率
				if (lightPdf == 0) return Ld;		//否则从bsdf采样失败，返回Ld
				weight = power_heuristic(1, scatteringPdf, 1, lightPdf);
			}

			// Find intersection and compute transmittance
			SurfaceInteraction lightInteraction;
			Ray ray = it.generate_ray(wi);
			Spectrum Tr(1.f);
			bool foundSurfaceInteraction =
				handleMedia ? scene.intersectTr(ray, sampler, &lightInteraction, &Tr)
				: scene.intersect(ray, &lightInteraction);

			// Add light contribution from material sampling
			Spectrum Li(0.f);
			if (foundSurfaceInteraction) 
			{
				if (lightInteraction.primitive->get_AreaLight() == &light)
					Li = lightInteraction.Le(-wi);	//调用SurInteraction的Le
			}
			else
				Li = light.Le(ray);		//调用light的Le
			if (!Li.is_black()) Ld += f * Li * Tr * weight / scatteringPdf;
		}
	}
	return Ld;
}

void check_radiance(int x, int y, Spectrum& L)
{
	// Issue warning if unexpected radiance value returned
	if (L.isnan())
	{
		LOG(ERROR) <<
			"Not-a-number radiance value returned "
			"for pixel( " << x << " , " << y << " ). Setting to black.";
		L = Spectrum(0.f);
	}
	else if (L.luminance() < -1e-5)
	{
		LOG(ERROR) <<
			"Negative luminance value returned "
			"for pixel( " << x << " , " << y << " ). Setting to black.";
		L = Spectrum(0.f);
	}
	else if (std::isinf(L.luminance()))
	{
		LOG(ERROR) <<
			"Infinite luminance value returned "
			"for pixel( " << x << " , " << y << " ). Setting to black.";
		L = Spectrum(0.f);
	}
}

// SamplerIntegrator Method Definitions
void SamplerIntegrator::render(const Scene &scene)
{
	preprocess(scene, *sampler);

	for (int y = 0; y < camera->film->height; ++y)
		for (int x = 0; x < camera->film->width; ++x)
		{
			for(int count = 0; count < sampler->samplesPerPixel; ++count)
			{ 
				Ray ray;
				CameraSample cs = sampler->get_CameraSample(x, y);
				camera->generate_ray(cs, &ray);

				Spectrum L = Li(ray, scene, *sampler);

				check_radiance(x, y, L);
				camera->film->add(cs.pFilm, L);
			}
		}
	camera->film->flush();
	//对一个像素采样n次，可否理解为长时间曝光
	//return camera->film->scale(1.f / sampler->samples_PerPixel);	//filter、flush
}

Spectrum SamplerIntegrator::specular_reflect(const Ray& ray, const SurfaceInteraction& isect,
	const Scene &scene, Sampler &sampler, int depth) const
{
	// Compute specular reflection direction _wi_ and BSDF value
	Vector3f wo = isect.wo, wi;
	Float pdf;
	BxDF_type type = BxDF_type::Reflection | BxDF_type::Specular;
	Spectrum f = isect.bsdf->sample_f(wo, &wi, sampler.get_2D(), &pdf, type);

	// Return contribution of specular reflection
	const Normal3f &ns = isect.shading.n;
	if (pdf > 0.f && !f.is_black() && AbsDot(wi, ns) != 0.f) 
	{
		// Compute ray differential _rd_ for specular reflection
		Ray rd = isect.generate_ray(wi);
		/*
		if (ray.hasDifferentials) 
		{
			rd.hasDifferentials = true;
			rd.rxOrigin = isect.p + isect.dpdx;
			rd.ryOrigin = isect.p + isect.dpdy;
			// Compute differential reflected directions
			Normal3f dndx = isect.shading.dndu * isect.dudx +
				isect.shading.dndv * isect.dvdx;
			Normal3f dndy = isect.shading.dndu * isect.dudy +
				isect.shading.dndv * isect.dvdy;
			Vector3f dwodx = -ray.rxDirection - wo,
				dwody = -ray.ryDirection - wo;
			Float dDNdx = Dot(dwodx, ns) + Dot(wo, dndx);
			Float dDNdy = Dot(dwody, ns) + Dot(wo, dndy);
			rd.rxDirection =
				wi - dwodx + 2.f * Vector3f(Dot(wo, ns) * dndx + dDNdx * ns);
			rd.ryDirection =
				wi - dwody + 2.f * Vector3f(Dot(wo, ns) * dndy + dDNdy * ns);
		}
		*/
		return f * Li(rd, scene, sampler, depth + 1) * AbsDot(wi, ns) /
			pdf;
	}
	else
		return Spectrum(0.f);
}

Spectrum SamplerIntegrator::specular_transmit(const Ray& ray, const SurfaceInteraction& isect,
	const Scene &scene, Sampler &sampler, int depth) const
{
	Vector3f wo = isect.wo, wi;
	Float pdf;
	const Point3f &p = isect.p;
	const Normal3f &ns = isect.shading.n;
	const BSDF &bsdf = *isect.bsdf;
	BxDF_type type = BxDF_type::Transmission | BxDF_type::Specular;

	Spectrum f = bsdf.sample_f(wo, &wi, sampler.get_2D(), &pdf, type);
	Spectrum L = Spectrum(0.f);

	if (pdf > 0.f && !f.is_black() && AbsDot(wi, ns) != 0.f) 
	{
		// Compute ray differential _rd_ for specular transmission
		Ray rd = isect.generate_ray(wi);
		/*
		if (ray.hasDifferentials) 
		{
			rd.hasDifferentials = true;
			rd.rxOrigin = p + isect.dpdx;
			rd.ryOrigin = p + isect.dpdy;

			Float eta = bsdf.eta;
			Vector3f w = -wo;
			if (Dot(wo, ns) < 0) eta = 1.f / eta;

			Normal3f dndx = isect.shading.dndu * isect.dudx +
				isect.shading.dndv * isect.dvdx;
			Normal3f dndy = isect.shading.dndu * isect.dudy +
				isect.shading.dndv * isect.dvdy;

			Vector3f dwodx = -ray.rxDirection - wo,
				dwody = -ray.ryDirection - wo;
			Float dDNdx = Dot(dwodx, ns) + Dot(wo, dndx);
			Float dDNdy = Dot(dwody, ns) + Dot(wo, dndy);

			Float mu = eta * Dot(w, ns) - Dot(wi, ns);
			Float dmudx =
				(eta - (eta * eta * Dot(w, ns)) / Dot(wi, ns)) * dDNdx;
			Float dmudy =
				(eta - (eta * eta * Dot(w, ns)) / Dot(wi, ns)) * dDNdy;

			rd.rxDirection =
				wi + eta * dwodx - Vector3f(mu * dndx + dmudx * ns);
			rd.ryDirection =
				wi + eta * dwody - Vector3f(mu * dndy + dmudy * ns);
		}
		*/
		L = f * Li(rd, scene, sampler, depth + 1) * AbsDot(wi, ns) / pdf;
	}
	return L;
}

}	//namespace valley