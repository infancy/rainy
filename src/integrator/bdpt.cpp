#include"bdpt.h"

namespace valley
{

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
	ScopedAssignment<Vertex> a1;
	if (s == 1)
		a1 = { qs, sampled };
	else if (t == 1)
		a1 = { pt, sampled };

	// Mark connection vertices as non-degenerate
	ScopedAssignment<bool> a2, a3;
	if (pt) a2 = { &pt->delta, false };
	if (qs) a3 = { &qs->delta, false };

	// Update reverse density of vertex $\pt{}_{t-1}$
	ScopedAssignment<Float> a4;
	if (pt)
		a4 = { &pt->pdfRev, s > 0 ? qs->Pdf(scene, qsMinus, *pt)
		: pt->PdfLightOrigin(scene, *ptMinus, lightPdf,
			lightToIndex) };

	// Update reverse density of vertex $\pt{}_{t-2}$
	ScopedAssignment<Float> a5;
	if (ptMinus)
		a5 = { &ptMinus->pdfRev, s > 0 ? pt->Pdf(scene, qs, *ptMinus)
		: pt->PdfLight(scene, *ptMinus) };

	// Update reverse density of vertices $\pq{}_{s-1}$ and $\pq{}_{s-2}$
	ScopedAssignment<Float> a6;
	if (qs) a6 = { &qs->pdfRev, pt->Pdf(scene, ptMinus, *qs) };
	ScopedAssignment<Float> a7;
	if (qsMinus) a7 = { &qsMinus->pdfRev, qs->Pdf(scene, pt, *qsMinus) };

	// Consider hypothetical connection strategies along the camera subpath
	Float ri = 1;
	for (int i = t - 1; i > 0; --i) {
		ri *=
			remap0(cameraVertices[i].pdfRev) / remap0(cameraVertices[i].pdfFwd);
		if (!cameraVertices[i].delta && !cameraVertices[i - 1].delta)
			sumRi += ri;
	}

	// Consider hypothetical connection strategies along the light subpath
	ri = 1;
	for (int i = s - 1; i >= 0; --i) {
		ri *= remap0(lightVertices[i].pdfRev) / remap0(lightVertices[i].pdfFwd);
		bool deltaLightvertex = i > 0 ? lightVertices[i - 1].delta
			: lightVertices[0].IsDeltaLight();
		if (!lightVertices[i].delta && !deltaLightvertex) sumRi += ri;
	}
	return 1 / (1 + sumRi);
}

int random_walk(const Scene &scene, RayDifferential ray, Sampler &sampler,
	Color beta, Float pdf, int maxDepth,
	TransportMode mode, Vertex *path) {
	if (maxDepth == 0) return 0;
	int bounces = 0;
	// Declare variables for forward and reverse probability densities
	Float pdfFwd = pdf, pdfRev = 0;
	while (true) {
		// Attempt to create the next subpath vertex in _path_
		MediumInteraction mi;

		VLOG(2) << "Random walk. Bounces " << bounces << ", beta " << beta <<
			", pdfFwd " << pdfFwd << ", pdfRev " << pdfRev;
		// Trace a ray and sample the medium, if any
		SurfaceInteraction isect;
		bool foundIntersection = scene.Intersect(ray, &isect);
		if (ray.medium) beta *= ray.medium->Sample(ray, sampler, arena, &mi);
		if (beta.IsBlack()) break;
		Vertex &vertex = path[bounces], &prev = path[bounces - 1];
		if (mi.IsValid()) {
			// Record medium interaction in _path_ and compute forward density
			vertex = Vertex::CreateMedium(mi, beta, pdfFwd, prev);
			if (++bounces >= maxDepth) break;

			// Sample direction and compute reverse density at preceding vertex
			Vector3f wi;
			pdfFwd = pdfRev = mi.phase->Sample_p(-ray.d, &wi, sampler.Get2D());
			ray = mi.SpawnRay(wi);
		}
		else {
			// Handle surface interaction for path generation
			if (!foundIntersection) {
				// Capture escaped rays when tracing from the camera
				if (mode == TransportMode::Radiance) {
					vertex = Vertex::CreateLight(EndpointInteraction(ray), beta,
						pdfFwd);
					++bounces;
				}
				break;
			}

			// Compute scattering functions for _mode_ and skip over medium
			// boundaries
			isect.ComputeScatteringFunctions(ray, arena, true, mode);
			if (!isect.bsdf) {
				ray = isect.SpawnRay(ray.d);
				continue;
			}

			// Initialize _vertex_ with surface intersection information
			vertex = Vertex::CreateSurface(isect, beta, pdfFwd, prev);
			if (++bounces >= maxDepth) break;

			// Sample BSDF at current vertex and compute reverse probability
			Vector3f wi, wo = isect.wo;
			BxDFType type;
			Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdfFwd,
				BSDF_ALL, &type);
			VLOG(2) << "Random walk sampled dir " << wi << " f: " << f <<
				", pdfFwd: " << pdfFwd;
			if (f.IsBlack() || pdfFwd == 0.f) break;
			beta *= f * AbsDot(wi, isect.shading.n) / pdfFwd;
			VLOG(2) << "Random walk beta now " << beta;
			pdfRev = isect.bsdf->Pdf(wi, wo, BSDF_ALL);
			if (type & BSDF_SPECULAR) {
				vertex.delta = true;
				pdfRev = pdfFwd = 0;
			}
			beta *= CorrectShadingNormal(isect, wo, wi, mode);
			VLOG(2) << "Random walk beta after shading normal correction " << beta;
			ray = isect.SpawnRay(wi);
		}

		// Compute reverse area density at preceding vertex
		prev.pdfRev = vertex.ConvertDensity(pdfRev, prev);
	}
	return bounces;
}

int generate_camera_subpath(const Scene &scene, Sampler &sampler, int maxDepth,
	Vertex *path, const Camera &camera, const Point2f &pFilm);

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

	Color Le = light->sample_Le(sampler.get_2D(), sampler.get_2D(), &ray,
		&nLight, &pdfPos, &pdfDir);
	if (pdfPos == 0 || pdfDir == 0 || Le.is_black()) return 0;

	// Generate first vertex on light subpath and start random walk
	path[0] = Vertex::create_light(light.get(), ray, nLight, Le, pdfPos * lightPdf);
		Color beta = Le * AbsDot(nLight, ray.d) / (lightPdf * pdfPos * pdfDir);

	VLOG(2) << "Starting light subpath. Ray: " << ray << ", Le " << Le <<
		", beta " << beta << ", pdfPos " << pdfPos << ", pdfDir " << pdfDir;

	int nVertices = random_walk(scene, ray, sampler, beta, pdfDir, 
		maxDepth - 1, TransportMode::Importance, path + 1);

	// Correct subpath sampling densities for infinite area lights
	if (path[0].IsInfiniteLight()) 
	{
		// Set spatial density of _path[1]_ for infinite area light
		if (nVertices > 0) {
			path[1].pdfFwd = pdfPos;
			if (path[1].IsOnSurface())
				path[1].pdfFwd *= AbsDot(ray.d, path[1].ng());
		}

		// Set spatial density of _path[0]_ for infinite area light
		path[0].pdfFwd =
			InfiniteLightDensity(scene, lightDistr, lightToIndex, ray.d);
	}
	return nVertices + 1;
}

Color connect_vertex(
	const Scene& scene, Vertex* lightVertices, Vertex* cameraVertices, int s,
	int t, const Distribution1D& lightDistr,
	const std::unordered_map<const Light*, size_t>& lightToIndex,
	const Camera& camera, Sampler& sampler, Point2f* pRaster,
	Float* misWeight = nullptr)
{
	Color L(0.f);
	// Ignore invalid connections related to infinite area lights
	if (t > 1 && s != 0 && cameraVertices[t - 1].type == VertexType::Light)
		return Color(0.f);

	// Perform connection and write contribution to _L_
	Vertex sampled;
	if (s == 0)
	{
		// Interpret the camera subpath as a complete path
		const Vertex& pt = cameraVertices[t - 1];
		if (pt.IsLight()) L = pt.Le(scene, cameraVertices[t - 2]) * pt.beta;
		DCHECK(!L.isnan());
	}
	else if (t == 1) 
	{
		// Sample a point on the camera and connect it to the light subpath
		const Vertex &qs = lightVertices[s - 1];
		if (qs.IsConnectible()) 
		{
			Visibility vis;
			Vector3f wi;
			Float pdf;
			Color Wi = camera.sample_Wi(qs.GetInteraction(), sampler.Get2D(),
				&wi, &pdf, pRaster, &vis);
			if (pdf > 0 && !Wi.IsBlack()) {
				// Initialize dynamically sampled vertex and _L_ for $t=1$ case
				sampled = Vertex::CreateCamera(&camera, vis.P1(), Wi / pdf);
				L = qs.beta * qs.f(sampled, TransportMode::Importance) * sampled.beta;
				if (qs.IsOnSurface()) L *= AbsDot(wi, qs.ns());
				DCHECK(!L.isnan());
				// Only check visibility after we know that the path would
				// make a non-zero contribution.
				if (!L.IsBlack()) L *= vis.Tr(scene, sampler);
			}
		}
	}
	else if (s == 1)
	{
		// Sample a point on a light and connect it to the camera subpath
		const Vertex &pt = cameraVertices[t - 1];
		if (pt.IsConnectible()) 
		{
			Float lightPdf;
			Visibility vis;
			Vector3f wi;
			Float pdf;
			int lightNum =
				lightDistr.SampleDiscrete(sampler.get_1D(), &lightPdf);
			const std::shared_ptr<Light> &light = scene.lights[lightNum];
			Color lightWeight = light->Sample_Li(
				pt.GetInteraction(), sampler.Get2D(), &wi, &pdf, &vis);
			if (pdf > 0 && !lightWeight.IsBlack())
			{
				EndpointInteraction ei(vis.P1(), light.get());
				sampled =
					Vertex::CreateLight(ei, lightWeight / (pdf * lightPdf), 0);
				sampled.pdfFwd =
					sampled.PdfLightOrigin(scene, pt, lightDistr, lightToIndex);
				L = pt.beta * pt.f(sampled, TransportMode::Radiance) * sampled.beta;
				if (pt.IsOnSurface()) L *= AbsDot(wi, pt.ns());
				// Only check visibility if the path would carry radiance.
				if (!L.IsBlack()) L *= vis.Tr(scene, sampler);
			}
		}
	}
	else
	{
		// Handle all other bidirectional connection cases
		const Vertex &qs = lightVertices[s - 1], &pt = cameraVertices[t - 1];
		if (qs.IsConnectible() && pt.IsConnectible()) {
			L = qs.beta * qs.f(pt, TransportMode::Importance) * pt.f(qs, TransportMode::Radiance) * pt.beta;
			VLOG(2) << "General connect s: " << s << ", t: " << t <<
				" qs: " << qs << ", pt: " << pt << ", qs.f(pt): " << qs.f(pt, TransportMode::Importance) <<
				", pt.f(qs): " << pt.f(qs, TransportMode::Radiance) << ", G: " << G(scene, sampler, qs, pt) <<
				", dist^2: " << DistanceSquared(qs.p(), pt.p());
			if (!L.IsBlack()) L *= G(scene, sampler, qs, pt);
		}
	}

	//if (L.IsBlack()) ++zeroRadiancePaths;

	// Compute MIS weight for connection strategy
	Float misWeight =
		L.is_black() ? 0.f : MISWeight(scene, lightVertices, cameraVertices,
			sampled, s, t, lightDistr, lightToIndex);
	VLOG(2) << "MIS weight for (s,t) = (" << s << ", " << t << ") connection: "
		<< misWeight;
	DCHECK(!std::isnan(misWeight));
	L *= misWeight;
	if (misWeightPtr) *misWeightPtr = misWeight;
	return L;
}

void BDPT::render(const Scene& scene)
{
	lightDistribution = std::unique_ptr<LightDistribution>{ new PowerDistribution(scene) };

	//从光源指针到 scene.lights/lightDistr 中的偏移量的映射
	std::unordered_map<const Light*, size_t> lightToIndex;
	for (size_t i = 0; i < scene.lights.size(); ++i)
		lightToIndex[scene.lights[i].get()] = i;

	for (int y = 0; y < camera->film->height; ++y)
		for (int x = 0; x < camera->film->width; ++x)
		{
			sampler->start_pixel(Point2i(x, y));
			do
			{
				Point2f pFilm = Point2f(x, y) + sampler->get_2D();

				// Trace the camera subpath
				Vertex* cameraVertices = new Vertex(maxDepth + 2);
				Vertex* lightVertices = new Vertex(maxDepth + 1);

				int nCamera = generate_camera_subpath(scene, *sampler, maxDepth + 2,
					cameraVertices, *camera, pFilm);

				//对光源路径使用基于相机定点的采样分布不够好
				const Distribution1D *lightDistr =
					lightDistribution->lookup(cameraVertices[0].p());
				// Now trace the light subpath
				int nLight = generate_light_subpath(
					scene, *sampler, maxDepth + 1, lightVertices,
					*lightDistr, lightToIndex);

				Color L(0.f);
				for (int t = 1; t <= nCamera; ++t)
					for (int s = 0; s <= nLight; ++s)
					{
						int depth = t + s - 2;
						if ((s == 1 && t == 1) || depth < 0 ||
							depth > maxDepth)
							continue;
						// Execute the $(s, t)$ connection strategy and
						// update _L_
						Point2f pFilmNew = pFilm;
						Float misWeight = 0.f;

						Color Lpath = connect_vertex(
							scene, lightVertices, cameraVertices, s, t,
							*lightDistr, lightToIndex, *camera, *sampler,
							&pFilmNew, &misWeight);
						VLOG(2) << "Connect bdpt s: " << s << ", t: " << t <<
							", Lpath: " << Lpath << ", misWeight: " << misWeight;

						L += Lpath;
					}

				VLOG(2) << "Add film sample pFilm: " << pFilm << ", L: " << L <<
					", (y: " << L.luminance() << ")";

				camera->film->add(pFilm, L);
			} while (sampler->next_sample());
		}
	camera->film->flush();
}

}	//namespace valley