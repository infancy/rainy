#include"bdpt.h"

namespace valley
{

// BDPT Utility Functions
Float InfiniteLight_density(
	const Scene &scene, const Distribution1D &lightDistr, const Vector3f &w,
	const std::unordered_map<const Light *, size_t> &lightToDistrIndex)
{
	Float pdf = 0;
	for (const auto &light : scene.infiniteLights) 
	{
		CHECK(lightToDistrIndex.find(light.get()) != lightToDistrIndex.end());
		size_t index = lightToDistrIndex.find(light.get())->second;
		pdf += light->pdf_Li(Interaction(), -w) * lightDistr.func[index];
	}
	return pdf / (lightDistr.funcInt * lightDistr.count());
}

Float correct_ShadingNormal(const SurfaceInteraction &isect, const Vector3f &wo,
	const Vector3f &wi, TransportMode mode) 
{
	if (mode == TransportMode::Importance) 
	{
		Float num = AbsDot(wo, isect.shading.n) * AbsDot(wi, isect.n);
		Float denom = AbsDot(wo, isect.n) * AbsDot(wi, isect.shading.n);
		//wi 可能与法线垂直
		if (denom == 0) return 0;
		return num / denom;
	}
	else
		return 1;
}

Spectrum G(const Scene &scene, Sampler &sampler, const Vertex &v0,
	const Vertex &v1) 
{
	Vector3f d = v0.p() - v1.p();
	Float g = 1 / d.LengthSquared();
	d *= std::sqrt(g);
	if (v0.on_surface()) g *= AbsDot(v0.ns(), d);
	if (v1.on_surface()) g *= AbsDot(v1.ns(), d);
	Visibility vis(v0.get_interaction(), v1.get_interaction());
	return g * vis.Tr(scene, sampler);
}

template <typename Type>
class Simulate	//伪装、替身
{
public:
	// Simulate Public Methods
	Simulate(Type *target = nullptr, Type image = Type())
		: target(target) 
	{
		if (target) 
		{
			backup = *target;
			*target = image;
		}
	}
	~Simulate() {
		if (target) *target = backup;
	}
	Simulate(const Simulate &) = delete;
	Simulate &operator=(const Simulate &) = delete;
	Simulate &operator=(Simulate &&other) 
	{
		target = other.target;
		backup = other.backup;
		other.target = nullptr;
		return *this;
	}

private:
	Type *target, backup;
};

Float mis_weight(const Scene &scene, Vertex *lightVertices,
	Vertex *cameraVertices, Vertex &sampled, int s, int t,
	const Distribution1D &lightPdf,
	const std::unordered_map<const Light *, size_t> &lightToIndex) 
{
	if (s + t == 2) return 1;
	Float sumRi = 0;
	// Define helper function _remap0_ that deals with Dirac delta functions
	auto remap0 = [](Float f) -> Float { return f != 0 ? f : 1; };

	// Temporarily update vertex properties for current strategy

	// Look up connection vertices and their predecessors
	Vertex *qs = s > 0 ? &lightVertices[s - 1] : nullptr,
		*pt = t > 0 ? &cameraVertices[t - 1] : nullptr,
		*qsMinus = s > 1 ? &lightVertices[s - 2] : nullptr,
		*ptMinus = t > 1 ? &cameraVertices[t - 2] : nullptr;

	// Update sampled vertex for $s=1$ or $t=1$ strategy
	Simulate<Vertex> a1;
	if (s == 1)
		a1 = { qs, sampled };
	else if (t == 1)
		a1 = { pt, sampled };

	// Mark connection vertices as non-degenerate
	Simulate<bool> a2, a3;
	if (pt) a2 = { &pt->delta, false };
	if (qs) a3 = { &qs->delta, false };

	// Update reverse density of vertex $\pt{}_{t-1}$
	// 在随机游走时，子路径pt、pt-1、qs、qs-1都是未经计算的（因为一生成末端顶点就退出循环了）
	Simulate<Float> a4;
	if (pt)
		a4 = { &pt->pdfRev, s > 0 ? qs->pdf(scene, qsMinus, *pt)
		: pt->pdf_LightOrigin(scene, *ptMinus, lightPdf,
			lightToIndex) };

	// Update reverse density of vertex $\pt{}_{t-2}$
	Simulate<Float> a5;
	if (ptMinus)
		a5 = { &ptMinus->pdfRev, s > 0 ? pt->pdf(scene, qs, *ptMinus)
		: pt->pdf_Light(scene, *ptMinus) };

	// Update reverse density of vertices $\pq{}_{s-1}$ and $\pq{}_{s-2}$
	Simulate<Float> a6;
	if (qs) a6 = { &qs->pdfRev, pt->pdf(scene, ptMinus, *qs) };
	Simulate<Float> a7;
	if (qsMinus) a7 = { &qsMinus->pdfRev, qs->pdf(scene, pt, *qsMinus) };

	// Consider hypothetical connection strategies along the camera subpath
	Float ri = 1;
	for (int i = t - 1; i > 0; --i)
	{
		ri *= remap0(cameraVertices[i].pdfRev) / remap0(cameraVertices[i].pdfFwd);
		if (!cameraVertices[i].delta && !cameraVertices[i - 1].delta)
			sumRi += ri;
	}

	// Consider hypothetical connection strategies along the light subpath
	ri = 1;
	for (int i = s - 1; i >= 0; --i) 
	{
		ri *= remap0(lightVertices[i].pdfRev) / remap0(lightVertices[i].pdfFwd);
		bool deltaLightvertex = i > 0 ? 
			lightVertices[i - 1].delta
			: lightVertices[0].is_DeltaLight();
		if (!lightVertices[i].delta && !deltaLightvertex) sumRi += ri;
	}
	return 1 / (1 + sumRi);
}

int random_walk(const Scene &scene, RayDifferential ray, Sampler &sampler,
	Spectrum beta, Float pdf, int maxDepth,
	TransportMode mode, Vertex *path) 
{
	if (maxDepth == 0) return 0;
	int bounces = 0;
	// Declare variables for forward and reverse probability densities
	Float pdfFwd = pdf, pdfRev = 0;
	while (true) 
	{
		// Attempt to create the next subpath vertex in _path_
		//MediumInteraction mi;

		VLOG(2) << "Random walk. Bounces " << bounces << ", beta " << beta <<
			", pdfFwd " << pdfFwd << ", pdfRev " << pdfRev;
		// Trace a ray and sample the medium, if any
		SurfaceInteraction isect;
		bool foundIntersection = scene.intersect(ray, &isect);
		//if (ray.medium) beta *= ray.medium->Sample(ray, sampler, arena, &mi);
		if (beta.is_black()) break;
		Vertex &vertex = path[bounces], &prev = path[bounces - 1];
		/*
		if (mi.IsValid()) 
		{
			// Record medium interaction in _path_ and compute forward density
			vertex = Vertex::CreateMedium(mi, beta, pdfFwd, prev);
			if (++bounces >= maxDepth) break;

			// Sample direction and compute reverse density at preceding vertex
			Vector3f wi;
			pdfFwd = pdfRev = mi.phase->Sample_p(-ray.d, &wi, sampler.Get2D());
			ray = mi.SpawnRay(wi);
		}
		else 
		*/
		{
			// Handle surface interaction for path generation
			if (!foundIntersection) 
			{
				// Capture escaped rays when tracing from the camera
				if (mode == TransportMode::Radiance) 
				{
					vertex = Vertex::create_light(EndpointInteraction(ray), beta,
						pdfFwd);
					++bounces;
				}
				break;
			}

			// Compute scattering functions for _mode_ and skip over medium
			// boundaries
			isect.compute_scattering(ray, mode, true);
			if (!isect.bsdf) 
			{
				ray = isect.generate_ray(ray.d);
				continue;
			}

			// Initialize _vertex_ with surface intersection information
			vertex = Vertex::create_surface(isect, beta, pdfFwd, prev);//生成顶点
			if (++bounces >= maxDepth) break;

			// Sample BSDF at current vertex and compute reverse probability
			Vector3f wi, wo = isect.wo;
			BxDFType type;

			Spectrum f = isect.bsdf->sample_f(wo, &wi, sampler.get_2D(), &pdfFwd, //wi->isect->wo
				BxDFType::All, &type);
			VLOG(2) << "Random walk sampled dir " << wi << " f: " << f <<
				", pdfFwd: " << pdfFwd;
			if (f.is_black() || pdfFwd == 0.f) break;
			beta *= f * AbsDot(wi, isect.shading.n) / pdfFwd;
			VLOG(2) << "Random walk beta now " << beta;

			pdfRev = isect.bsdf->pdf(wi, wo, BxDFType::All);	//wo->isect->wi,即反向概率
			if (static_cast<bool>(type & BxDFType::Specular)) 
			{
				vertex.delta = true;
				pdfRev = pdfFwd = 0;
			}
			beta *= correct_ShadingNormal(isect, wo, wi, mode);
			VLOG(2) << "Random walk beta after shading normal correction " << beta << "\n\n";
			ray = isect.generate_ray(wi);
		}

		// Compute reverse area density at preceding vertex
		prev.pdfRev = vertex.convert_density(pdfRev, prev);
	}
	VLOG(2) << "\n\n";
	return bounces;
}

int generate_camera_subpath(const Scene &scene, Sampler &sampler, int maxDepth,
	Vertex *path, const Camera &camera, const Point2f &pFilm)
{
	if (maxDepth == 0) return 0;

	// Sample initial ray for camera subpath
	CameraSample cameraSample;
	cameraSample.pFilm = pFilm;
	cameraSample.pLens = sampler.get_2D();
	RayDifferential ray;
	Spectrum beta = camera.generate_ray(cameraSample, &ray);
	//ray.ScaleDifferentials(1 / std::sqrt(sampler.samplesPerPixel));

	// Generate first vertex on camera subpath and start random walk
	Float pdfPos, pdfDir;
	path[0] = Vertex::create_camera(&camera, ray, beta);
	camera.pdf_We(ray, &pdfPos, &pdfDir);

	VLOG(2) << "Starting camera subpath. Ray: " << ray << ", beta " << beta
		<< ", pdfPos " << pdfPos << ", pdfDir " << pdfDir << "\n\n";

	return random_walk(scene, ray, sampler, beta, pdfDir, maxDepth - 1,
		TransportMode::Radiance, path + 1) +
		1;
}

int generate_light_subpath(const Scene &scene, Sampler &sampler, int maxDepth,
	Vertex *path, const Distribution1D &lightDistr,
	const std::unordered_map<const Light *, size_t> &lightToIndex)
{
	if (maxDepth == 0) return 0;

	// Sample initial ray for light subpath
	Float lightPdf;
	int lightNum = lightDistr.sample_discrete(sampler.get_1D(), &lightPdf);
	const std::shared_ptr<Light>& light = scene.lights[lightNum];

	RayDifferential ray;
	Normal3f nLight;
	Float pdfPos, pdfDir;

	Spectrum Le = light->sample_Le(sampler.get_2D(), sampler.get_2D(), &ray,
		&nLight, &pdfPos, &pdfDir);
	if (pdfPos == 0 || pdfDir == 0 || Le.is_black()) return 0;

	// Generate first vertex on light subpath and start random walk
	path[0] = Vertex::create_light(light.get(), ray, nLight, Le, pdfPos * lightPdf);
		Spectrum beta = Le * AbsDot(nLight, ray.d) / (lightPdf * pdfPos * pdfDir);

	VLOG(2) << "Starting light subpath. Ray: " << ray << ", Le " << Le <<
		", beta " << beta << ", pdfPos " << pdfPos << ", pdfDir " << pdfDir << "\n\n";

	int nVertices = random_walk(scene, ray, sampler, beta, pdfDir, 
		maxDepth - 1, TransportMode::Importance, path + 1);

	// Correct subpath sampling densities for infinite area lights
	if (path[0].is_InfiniteLight()) 
	{
		// Set spatial density of _path[1]_ for infinite area light
		if (nVertices > 0) 
		{
			path[1].pdfFwd = pdfPos;
			if (path[1].on_surface())
				path[1].pdfFwd *= AbsDot(ray.d, path[1].ng());
		}

		// Set spatial density of _path[0]_ for infinite area light
		path[0].pdfFwd =
			InfiniteLight_density(scene, lightDistr, ray.d, lightToIndex);
	}
	return nVertices + 1;
}

Spectrum connect_vertex(
	const Scene& scene, Vertex* lightVertices, Vertex* cameraVertices, int s,
	int t, const Distribution1D& lightDistr,
	const std::unordered_map<const Light*, size_t>& lightToIndex,
	const Camera& camera, Sampler& sampler, Point2f* pRaster,
	Float* misWeight_ptr)
{
	Spectrum L(0.f);
	// Ignore invalid connections related to infinite area lights
	if (t > 1 && s != 0 && cameraVertices[t - 1].type == VertexType::Light)
		return Spectrum(0.f);

	// Perform connection and write contribution to _L_
	Vertex sampled;
	if (s == 0)	//当 maxDepth 为 0 时，s 为 0，此时相当于 pathTracing
	{
		// 此时如果相机子路径连接到了光源，则可生成一条完整的路径
		const Vertex& pt = cameraVertices[t - 1];
		if (pt.is_Light()) L = pt.Le(scene, cameraVertices[t - 2]) * pt.beta;	//infinite & area??
		DCHECK(!L.isnan());
	}
	else if (t == 1) 
	{
		// Sample a point on the camera and connect it to the light subpath
		const Vertex &qs = lightVertices[s - 1];
		if (qs.is_connectible()) 
		{
			Visibility vis;
			Vector3f wi;
			Float pdf;
			Spectrum Wi = camera.sample_Wi(qs.get_interaction(), sampler.get_2D(),
				&wi, &pdf, pRaster, &vis);
			if (pdf > 0 && !Wi.is_black()) 
			{
				// Initialize dynamically sampled vertex and _L_ for $t=1$ case
				sampled = Vertex::create_camera(&camera, vis.P1(), Wi / pdf);
				L = qs.beta * qs.f(sampled, TransportMode::Importance) * sampled.beta;
				if (qs.on_surface()) L *= AbsDot(wi, qs.ns());
				DCHECK(!L.isnan());
				// Only check visibility after we know that the path would
				// make a non-zero contribution.
				if (!L.is_black()) L *= vis.Tr(scene, sampler);
			}
		}
	}
	else if (s == 1)
	{
		// Sample a point on a light and connect it to the camera subpath
		const Vertex &pt = cameraVertices[t - 1];
		if (pt.is_connectible()) 
		{
			Float lightPdf;
			Visibility vis;
			Vector3f wi;
			Float pdf;
			int lightNum =
				lightDistr.sample_discrete(sampler.get_1D(), &lightPdf);
			const std::shared_ptr<Light> &light = scene.lights[lightNum];
			Spectrum lightWeight = light->sample_Li(
				pt.get_interaction(), sampler.get_2D(), &wi, &pdf, &vis);
			if (pdf > 0 && !lightWeight.is_black())
			{
				EndpointInteraction ei(vis.P1(), light.get());
				sampled =
					Vertex::create_light(ei, lightWeight / (pdf * lightPdf), 0);
				sampled.pdfFwd =
					sampled.pdf_LightOrigin(scene, pt, lightDistr, lightToIndex);
				L = pt.beta * pt.f(sampled, TransportMode::Radiance) * sampled.beta;
				if (pt.on_surface()) L *= AbsDot(wi, pt.ns());
				// Only check visibility if the path would carry radiance.
				if (!L.is_black()) L *= vis.Tr(scene, sampler);
			}
		}
	}
	else
	{
		// Handle all other bidirectional connection cases
		const Vertex &qs = lightVertices[s - 1], &pt = cameraVertices[t - 1];
		if (qs.is_connectible() && pt.is_connectible()) 
		{
			//qs.beta 记录的是 qs-1 的值，而 qs 与 pt 间的几何项稍后计算
			L = qs.beta * qs.f(pt, TransportMode::Importance) * pt.f(qs, TransportMode::Radiance) * pt.beta;

			/*
			VLOG(2) << "General connect s: " << s << ", t: " << t <<
				" qs: " << qs << ", pt: " << pt << ", qs.f(pt): " << qs.f(pt, TransportMode::Importance) <<
				", pt.f(qs): " << pt.f(qs, TransportMode::Radiance) << ", G: " << G(scene, sampler, qs, pt) <<
				", dist^2: " << DistanceSquared(qs.p(), pt.p());
			*/

			if (!L.is_black()) L *= G(scene, sampler, qs, pt);
		}
	}

	//if (L.IsBlack()) ++zeroRadiancePaths;

	// Compute MIS weight for connection strategy
	
	Float misWeight =
		L.is_black() ? 0.f : mis_weight(scene, lightVertices, cameraVertices,
			sampled, s, t, lightDistr, lightToIndex);

	VLOG(2) << "MIS weight for (s,t) = (" << s << ", " << t << ") connection: "
		<< misWeight;

	DCHECK(!std::isnan(misWeight));

	L = L * misWeight;

	if (misWeight_ptr) *misWeight_ptr = misWeight;
	
	return L;
}

void BDPT::render(const Scene& scene)
{
	for (int y = 0; y < camera->film->height; ++y)
	{ 
		std::cout << y << '\n';
		for (int x = 0; x < camera->film->width; ++x)
		{
			sampler->start_pixel(Point2i(x, y));
			do
			{
				Point2f pFilm = Point2f(x, y) + sampler->get_2D();

				// Trace the camera subpath
				std::unique_ptr<Vertex[]> cameraVertices(new Vertex[cameraDepth + 1]);
				std::unique_ptr<Vertex[]> lightVertices(new Vertex[lightDepth + 1]);

				int nCamera = generate_camera_subpath(scene, *sampler, cameraDepth + 1,
					cameraVertices.get(), *camera, pFilm);

				//对光源路径使用基于相机顶点的采样分布不够好
				const Distribution1D* lightDistr =
					lightDistribution->lookup(cameraVertices[0].p());

				int nLight = generate_light_subpath(scene, *sampler, lightDepth + 1, 
					lightVertices.get(), *lightDistr, lightToIndex);

				Spectrum L(0.f);
				for (int t = 1; t <= nCamera; ++t)
					for (int s = 0; s <= nLight; ++s)
					{
						int depth = t + s - 2;
						if ((s == 1 && t == 1) || depth < 0 ||
							depth > pathDepth)
							continue;
						// Execute the $(s, t)$ connection strategy and
						// update _L_
						Point2f pFilmNew = pFilm;
						Float misWeight = 0.f;

						Spectrum Lpath = connect_vertex(
							scene, lightVertices.get(), cameraVertices.get(), s, t,
							*lightDistr, lightToIndex, *camera, *sampler,
							&pFilmNew, &misWeight);

						VLOG(2) << "Connect bdpt s: " << s << ", t: " << t <<
							", Lpath: " << Lpath << "\n\n";

						if (t != 1)
							L += Lpath;
						else
							//camera->film->add(pFilmNew, L);
							camera->film->add_splat(pFilmNew, Lpath);
					}

				VLOG(2) << "Add film sample pFilm: " << pFilm << ", L: " << L <<
					", (y: " << L.luminance() << ")";

				camera->film->add(pFilm, L);
			} while (sampler->next_sample());
		}
	}
	camera->film->flush();
}

void BDPT::interactive(const Scene& scene, int x, int y)
{
	Point2f pFilm = Point2f(x, y) + sampler->get_2D();

	// Trace the camera subpath
	std::unique_ptr<Vertex[]> cameraVertices(new Vertex[cameraDepth + 1]);
	std::unique_ptr<Vertex[]> lightVertices(new Vertex[lightDepth + 1]);

	int nCamera = generate_camera_subpath(scene, *sampler, cameraDepth + 1,
		cameraVertices.get(), *camera, pFilm);

	//对光源路径使用基于相机顶点的采样分布不够好
	const Distribution1D* lightDistr =
		lightDistribution->lookup(cameraVertices[0].p());

	int nLight = generate_light_subpath(scene, *sampler, lightDepth + 1,
		lightVertices.get(), *lightDistr, lightToIndex);

	Spectrum L(0.f);
	for (int t = 1; t <= nCamera; ++t)
		for (int s = 0; s <= nLight; ++s)
		{
			int depth = t + s - 2;
			if ((s == 1 && t == 1) || depth < 0 ||
				depth > pathDepth)
				continue;
			// Execute the $(s, t)$ connection strategy and
			// update _L_
			Point2f pFilmNew = pFilm;
			Float misWeight = 0.f;

			Spectrum Lpath = connect_vertex(
				scene, lightVertices.get(), cameraVertices.get(), s, t,
				*lightDistr, lightToIndex, *camera, *sampler,
				&pFilmNew, &misWeight);

			VLOG(2) << "Connect bdpt s: " << s << ", t: " << t <<
				", Lpath: " << Lpath << "\n\n";
		}
}

}	//namespace valley