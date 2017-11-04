#include"sppm.h"

namespace rainy
{

struct SPPMPixel 
{
	SPPMPixel() : M(0) {}

	Float radius = 0;
	Spectrum Ld;
	Spectrum Phi;
	int M;
	Float N = 0;
	Spectrum tau;

	struct VisiblePoint
	{
		VisiblePoint() {}
		VisiblePoint(const Point3f &p, const Vector3f &wo, const BSDF *bsdf,
			const Spectrum &beta)
			: p(p), wo(wo), bsdf(bsdf), beta(beta) {}

		Point3f p;
		Vector3f wo;
		std::unique_ptr<const BSDF> bsdf;
		Spectrum beta;
	} vp;
};

struct SPPMPixelListNode 
{
	SPPMPixel* pixel;
	SPPMPixelListNode* next;
};

static bool ToGrid(const Point3f &p, const Bounds3f &bounds,
	const int gridRes[3], Point3i *pi)
{
	bool inBounds = true;
	Vector3f pg = bounds.Offset(p);
	for (int i = 0; i < 3; ++i) 
	{
		(*pi)[i] = (int)(gridRes[i] * pg[i]);
		inBounds &= ((*pi)[i] >= 0 && (*pi)[i] < gridRes[i]);
		(*pi)[i] = Clamp((*pi)[i], 0, gridRes[i] - 1);
	}
	return inBounds;
}

inline unsigned int hash(const Point3i &p, int hashSize) {
	return (unsigned int)((p.x * 73856093) ^ (p.y * 19349663) ^
		(p.z * 83492791)) %
		hashSize;
}

// SPPM Method Definitions
void SPPM::render(const Scene &scene)
{
	// Initialize _pixelBounds_ and _pixels_ array for SPPM
	Bounds2i pixelBounds = camera->film->bounds;
	int nPixels = pixelBounds.Area();
	std::unique_ptr<SPPMPixel[]> pixels(new SPPMPixel[nPixels]);
	for (int i = 0; i < nPixels; ++i) pixels[i].radius = initial_SearchRadius;

	const Float invSqrtSPP = 1.f / std::sqrt(nIterations);

	// Compute _lightDistr_ for sampling lights proportional to power
	std::unique_ptr<Distribution1D> lightDistr =
		light_power_distribution(scene);

	// Perform _nIterations_ of SPPM integration
	//HaltonSampler sampler(nIterations, pixelBounds);

	for (int iter = 0; iter < nIterations; ++iter) 
	{
		std::cout << "iter : " << iter << "\n";
		// Generate SPPM visible points
		for (int y = 0; y < camera->film->height; ++y)
		{	
			for (int x = 0; x < camera->film->width; ++x)
			{
				Point2i pPixel(x, y);
				// Prepare _sampler_ for _pPixel_
				sampler->start_pixel(pPixel);
				sampler->set_SampleIndex(iter);		//等同于对下一个采样点进行采样

				// Generate camera ray for pixel for SPPM
				CameraSample cameraSample =
					sampler->get_CameraSample(pPixel);
				RayDifferential ray;
				Spectrum beta = camera->generate_ray(cameraSample, &ray);
				//ray.ScaleDifferentials(invSqrtSPP);

				// Follow camera ray path until a visible point is created

				// Get _SPPMPixel_ for _pPixel_
				Point2i pPixelO = Point2i(pPixel - pixelBounds.pMin);
				int pixelOffset =
					pPixelO.x +
					pPixelO.y * (pixelBounds.pMax.x - pixelBounds.pMin.x);
				SPPMPixel &pixel = pixels[pixelOffset];

				bool specularBounce = false;
				for (int depth = 0; depth < maxDepth; ++depth) 
				{
					SurfaceInteraction isect;

					if (!scene.intersect(ray, &isect))
					{
						// Accumulate light contributions for ray with no
						// intersection
						for (const auto &light : scene.lights)
							pixel.Ld += beta * light->Le(ray);
						break;
					}
					// Process SPPM camera ray intersection

					// Compute BSDF at SPPM camera ray intersection
					isect.compute_scattering(ray);
					if (!isect.bsdf)
					{
						ray = isect.generate_ray(ray.d);
						--depth;
						continue;
					}
					const BSDF& bsdf = *isect.bsdf;

					// Accumulate direct illumination at SPPM camera ray
					// intersection
					Vector3f wo = -ray.d;
					if (depth == 0 || specularBounce)
						pixel.Ld += beta * isect.Le(wo);
					pixel.Ld +=
						beta * uniform_sample_one_light(isect, scene, *sampler);

					// Possibly create visible point and end camera path
					bool isDiffuse = bsdf.components_num(BxDFType::Diffuse | 
						BxDFType::Reflection | BxDFType::Transmission) > 0;
					bool isGlossy = bsdf.components_num(BxDFType::Glossy |
						BxDFType::Reflection | BxDFType::Transmission) > 0;

					if (isDiffuse || (isGlossy && depth == maxDepth - 1)) 
					{
						pixel.vp = { isect.p, wo, isect.bsdf.release(), beta };
						break;
					}

					// Spawn ray from SPPM camera path vertex
					if (depth < maxDepth - 1) 
					{
						Float pdf;
						Vector3f wi;
						BxDFType type;
						Spectrum f =
							bsdf.sample_f(wo, &wi, sampler->get_2D(),
								&pdf, BxDFType::All, &type);
						if (pdf == 0. || f.is_black()) break;
						specularBounce = (int)(type & BxDFType::Specular) != 0;
						beta *= f * AbsDot(wi, isect.shading.n) / pdf;
						if (beta.luminance() < 0.25) 
						{
							Float continueProb =
								std::min((Float)1, beta.luminance());
							if (sampler->get_1D() > continueProb) break;
							beta /= continueProb;
						}
						ray = (RayDifferential)isect.generate_ray(wi);
					}
				}
			}
		}


		// Create grid of all SPPM visible points
		int gridRes[3];
		Bounds3f gridBounds;
		// Allocate grid for SPPM visible points
		const int hashSize = nPixels;
		std::vector<SPPMPixelListNode*> grid(hashSize, nullptr);
		{
			// Compute grid bounds for SPPM visible points
			Float maxRadius = 0.;
			for (int i = 0; i < nPixels; ++i)
			{
				const SPPMPixel &pixel = pixels[i];
				if (pixel.vp.beta.is_black()) continue;
				Bounds3f vpBound = Expand(Bounds3f(pixel.vp.p), pixel.radius);

				gridBounds = Union(gridBounds, vpBound);	
				maxRadius = std::max(maxRadius, pixel.radius);
			}

			// Compute resolution of SPPM grid in each dimension
			Vector3f diag = gridBounds.Diagonal();
			Float maxDiag = MaxComponent(diag);
			int baseGridRes = (int)(maxDiag / maxRadius);
			CHECK_GT(baseGridRes, 0);
			for (int i = 0; i < 3; ++i)
				gridRes[i] = std::max((int)(baseGridRes * diag[i] / maxDiag), 1);

			// Add visible points to SPPM grid
			for(int pixelIndex = 0; pixelIndex < nPixels; ++pixelIndex)
			{
				SPPMPixel& pixel = pixels[pixelIndex];
				if (!pixel.vp.beta.is_black()) 
				{
					// Add pixel's visible point to applicable grid cells
					Float radius = pixel.radius;
					Point3i pMin, pMax;
					ToGrid(pixel.vp.p - Vector3f(radius, radius, radius),
						gridBounds, gridRes, &pMin);
					ToGrid(pixel.vp.p + Vector3f(radius, radius, radius),
						gridBounds, gridRes, &pMax);

					for (int z = pMin.z; z <= pMax.z; ++z)
						for (int y = pMin.y; y <= pMax.y; ++y)
							for (int x = pMin.x; x <= pMax.x; ++x) 
							{
								// Add visible point to grid cell $(x, y, z)$
								// from grid[h] find the visible points 
								int h = hash(Point3i(x, y, z), hashSize);
								//auto node = std::make_unique<SPPMPixelListNode>();
								auto node = new SPPMPixelListNode;
								node->pixel = &pixel;

								node->next = grid[h];
								while (true)
									if (grid[h] == node->next)
									{
										grid[h] = node;
										break;
									}
									else
										grid[h] = node->next;
							}
				}
			}
		}


		// Trace photons and accumulate contributions
		for (int photonIndex = 0; photonIndex < photons_PerIteration; ++photonIndex)
		{
			//MemoryArena &arena = photonShootArenas[ThreadIndex];
			// Follow photon path for _photonIndex_
			uint64_t haltonIndex =
				(uint64_t)iter * (uint64_t)photons_PerIteration + photonIndex;
			int haltonDim = 0;

			// Choose light to shoot photon from
			Float lightPdf;
			Float lightSample = sampler->get_1D();
			int lightNum = lightDistr->sample_discrete(lightSample, &lightPdf);
			const std::shared_ptr<Light> &light = scene.lights[lightNum];

			// Compute sample values for photon ray leaving light source
			Point2f uLight0 = sampler->get_2D(); 
			Point2f uLight1 = sampler->get_2D();

			haltonDim += 5;

			// Generate _photonRay_ from light source and initialize _beta_
			RayDifferential photonRay;
			Normal3f nLight;
			Float pdfPos, pdfDir;
			Spectrum Le =
				light->sample_Le(uLight0, uLight1, &photonRay,
					&nLight, &pdfPos, &pdfDir);
			if (pdfPos == 0 || pdfDir == 0 || Le.is_black()) return;
			Spectrum beta = (AbsDot(nLight, photonRay.d) * Le) /
				(lightPdf * pdfPos * pdfDir);
			if (beta.is_black()) return;

			// Follow photon path through scene and record intersections
			SurfaceInteraction isect;
			for (int depth = 0; depth < maxDepth; ++depth)
			{
				if (!scene.intersect(photonRay, &isect)) break;

				if (depth > 0) 
				{
					// Add photon contribution to nearby visible points
					Point3i photonGridIndex;
					if (ToGrid(isect.p, gridBounds, gridRes, &photonGridIndex)) 
					{
						int h = hash(photonGridIndex, hashSize);
						// Add photon contribution to visible points in
						// _grid[h]_
						for (SPPMPixelListNode *node = grid[h];
							node != nullptr; node = node->next)
						{
							SPPMPixel &pixel = *node->pixel;
							Float radius = pixel.radius;
							if (DistanceSquared(pixel.vp.p, isect.p) >
								radius * radius)
								continue;
							// Update _pixel_ $\Phi$ and $M$ for nearby
							// photon
							Vector3f wi = -photonRay.d;
							//VLOG(2) << "Phi : " << pixel.Phi << ", vp.bsdf : " << pixel.vp.bsdf;
							pixel.Phi += beta * pixel.vp.bsdf->f(pixel.vp.wo, wi);
							++pixel.M;
						}
					}
				}
				// Sample new photon ray direction

				// Compute BSDF at photon intersection point
				isect.compute_scattering(photonRay, TransportMode::Importance, true);
				if (!isect.bsdf) 
				{
					--depth;
					photonRay = isect.generate_ray(photonRay.d);
					continue;
				}
				const BSDF &photonBSDF = *isect.bsdf;

				// Sample BSDF _fr_ and direction _wi_ for reflected photon
				Vector3f wi, wo = -photonRay.d;
				Float pdf;
				BxDFType flags;

				// Generate _bsdfSample_ for outgoing photon sample
				Point2f bsdfSample = sampler->get_2D();
				haltonDim += 2;
				Spectrum fr = photonBSDF.sample_f(wo, &wi, bsdfSample, &pdf,
					BxDFType::All, &flags);
				if (fr.is_black() || pdf == 0.f) break;
				Spectrum bnew =
					beta * fr * AbsDot(wi, isect.shading.n) / pdf;

				// Possibly terminate photon path with Russian roulette
				Float q = std::max((Float)0, 1 - bnew.luminance() / beta.luminance());
				if (sampler->get_1D() < q) break;
				beta = bnew / (1 - q);
				photonRay = (RayDifferential)isect.generate_ray(wi);
			}
			//arena.Reset();
		}

		//delete grid->next
		for (auto g : grid)
		{
			SPPMPixelListNode *node = g, *next = nullptr;
			
			while (node)
			{
				next = node->next;
				delete node;
				node = next;
			}
		}


		// Update pixel values from this pass's photons
		for (int i = 0; i < nPixels; ++i)
		{
			SPPMPixel &p = pixels[i];
			if (p.M > 0)
			{
				// Update pixel photon count, search radius, and $\tau$ from
				// photons
				Float gamma = (Float)2 / (Float)3;
				Float Nnew = p.N + gamma * p.M;
				Float Rnew = p.radius * std::sqrt(Nnew / (p.N + p.M));
				Spectrum Phi = p.Phi;
				p.tau = (p.tau + p.vp.beta * Phi) * (Rnew * Rnew) /
					(p.radius * p.radius);
				p.N = Nnew;
				p.radius = Rnew;
				p.M = 0;
				p.Phi = 0;
			}
			// Reset _VisiblePoint_ in pixel
			p.vp.beta = 0.;
			p.vp.bsdf.reset(nullptr);
		}


		// Periodically store SPPM image in film and write image
		if (iter + 1 == nIterations || ((iter + 1) % writeFrequency) == 0)
		{
			int x0 = pixelBounds.pMin.x;
			int x1 = pixelBounds.pMax.x;
			uint64_t Np = (uint64_t)(iter + 1) * (uint64_t)photons_PerIteration;
			std::unique_ptr<Spectrum[]> image(new Spectrum[pixelBounds.Area()]);
			int offset = 0;
			for (int y = pixelBounds.pMin.y; y < pixelBounds.pMax.y; ++y) {
				for (int x = x0; x < x1; ++x) {
					// Compute radiance _L_ for SPPM pixel _pixel_
					const SPPMPixel &pixel =
						pixels[(y - pixelBounds.pMin.y) * (x1 - x0) + (x - x0)];
					Spectrum L = pixel.Ld / (iter + 1);
					L += pixel.tau / (Np * Pi * pixel.radius * pixel.radius);
					image[offset++] = L;
				}
			}
			camera->film->set_image(image.get());
			camera->film->flush();
		}
	}
}

}	//namespace rainy
