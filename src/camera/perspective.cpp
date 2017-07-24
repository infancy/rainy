#include"perspective.h"
#include"sampling.h"
#include"intersection.h"
#include"light.h"

namespace valley
{

PerspectiveCamera::PerspectiveCamera(const Point3f eye, const Point3f target, const Vector3f up,
	Float fovy, Film* film, Float lensRadius, Float focalDistance)
	: Camera(film), lensRadius(lensRadius), 
	focalDistance(focalDistance)
{
	//令近平面为z=1，则相当于Film在z=1的位置
	Float n = 1.f, f = 1000.f, aspect = film->width / film->height;
	Float tanAng = std::tan(Radians(fovy) / 2), cotAng = 1.f / tanAng;	//用于对x、y坐标进行缩放
	Area = 4 * tanAng  * tanAng * aspect;			//计算在z=1时screen的面积


	//raster_to_camera

	Matrix4x4 persp(1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, f / (f - n), -f * n / (f - n),
					0, 0, 1, 0);
	Transform camera_to_screen = Scale(cotAng / aspect, cotAng, 1) * Transform(persp);


	Matrix4x4 s2r(0.5 * film->width, 0,   0, 0.5 * film->width,
				  0, -0.5 * film->height, 0, 0.5 * film->height,
				  0, 0, 1, 0,
			      0, 0, 0, 1);
	Transform screen_to_raster(s2r);

	Transform camera_to_raster = screen_to_raster * camera_to_screen;
	//Transform raster_to_camera = Inverse(camera_to_raster);
	raster_to_camera = Inverse(camera_to_raster);


	//camera_to_world

	Vector3f forward = Normalize(target - eye);

	int parallel = 0;
	for (int i = 0; i < 3; ++i)
		if (up[i] == forward[i])
			++parallel;
	CHECK(parallel < 2);	//当 up 和 forward 平行时 right = (0,0,0)

	Vector3f right = Normalize(Cross(up, forward));			//左手叉乘
	Vector3f upward = Cross(forward, right);

	Matrix4x4 c2w(right.x, upward.x, forward.x, eye.x,
				  right.y, upward.y, forward.y, eye.y,
				  right.z, upward.z, forward.z, eye.z,
				  0, 0, 0, 1);
	camera_to_world = Transform(c2w);


	// world_to_raster

	world_to_raster = camera_to_raster * Inverse(camera_to_world);
}

Float PerspectiveCamera::generate_ray(const CameraSample& sample, Ray* ray) const
{
	Point3f pFilm(sample.pFilm.x, sample.pFilm.y, 0);
	Point3f pCamera = raster_to_camera(pFilm);

	*ray = Ray(Point3f(0, 0, 0), Vector3f(pCamera));

	// Modify ray for depth of field
	if (lensRadius > 0) 
	{
		// Sample point on lens
		Point2f pLens = lensRadius * concentric_sample_disk(sample.pLens);

		// Compute point on plane of focus
		Float ft = focalDistance / ray->d.z;
		Point3f pFocus = (*ray)(ft);

		// Update ray for effect of lens
		ray->o = Point3f(pLens.x, pLens.y, 0);
		ray->d = Normalize(pFocus - ray->o);
	}

	*ray = camera_to_world(*ray);

	return 1;
}

Color PerspectiveCamera::We(const Ray& ray, Point2f* pRaster2) const
{
	Float cosTheta = Dot(ray.d, camera_to_world(Vector3f(0, 0, 1)));
	if (cosTheta <= 0) return 0;

	// Map ray $(\p{}, \w{})$ onto the raster grid
	Point3f pFocus = ray((lensRadius > 0 ? focalDistance : 1) / cosTheta);
	Point3f pRaster = world_to_raster(pFocus);

	// Return raster position if requested
	if (pRaster2) *pRaster2 = Point2f(pRaster.x, pRaster.y);

	// Return zero importance for out of bounds points
	Bounds2i sampleBounds = film->get_sample_bounds();
	if (pRaster.x < sampleBounds.pMin.x || pRaster.x >= sampleBounds.pMax.x ||
		pRaster.y < sampleBounds.pMin.y || pRaster.y >= sampleBounds.pMax.y)
		return 0;

	// Compute lens area of perspective camera
	Float lensArea = lensRadius != 0 ? (Pi * lensRadius * lensRadius) : 1;

	// Return importance for point on image plane
	Float cos2Theta = cosTheta * cosTheta;
	return Color(1 / (Area * lensArea * cos2Theta * cos2Theta));
}

void PerspectiveCamera::pdf_We(const Ray& ray, Float* pdfPos, Float* pdfDir) const
{
	// Interpolate camera matrix and fail if $\w{}$ is not forward-facing
	Float cosTheta = Dot(ray.d, camera_to_world(Vector3f(0, 0, 1)));
	if (cosTheta <= 0) {
		*pdfPos = *pdfDir = 0;
		return;
	}

	// Map ray $(\p{}, \w{})$ onto the raster grid
	Point3f pFocus = ray((lensRadius > 0 ? focalDistance : 1) / cosTheta);
	Point3f pRaster = world_to_raster(pFocus);

	// Return zero probability for out of bounds points
	Bounds2i sampleBounds = film->get_sample_bounds();
	if (pRaster.x < sampleBounds.pMin.x || pRaster.x >= sampleBounds.pMax.x ||
		pRaster.y < sampleBounds.pMin.y || pRaster.y >= sampleBounds.pMax.y) {
		*pdfPos = *pdfDir = 0;
		return;
	}

	// Compute lens area of perspective camera
	Float lensArea = lensRadius != 0 ? (Pi * lensRadius * lensRadius) : 1;
	*pdfPos = 1 / lensArea;
	*pdfDir = 1 / (Area * cosTheta * cosTheta * cosTheta);
}

Color PerspectiveCamera::sample_Wi(const Isect& ref, const Point2f& u,
	Vector3f* wi, Float* pdf, Point2f* pRaster, Visibility* vis) const
{
	// Uniformly sample a lens interaction _lensIntr_
	Point2f pLens = lensRadius * concentric_sample_disk(u);
	Point3f pLensWorld = camera_to_world(Point3f(pLens.x, pLens.y, 0));
	Isect lensIntr(pLensWorld);
	lensIntr.n = Normal3f(camera_to_world(Vector3f(0, 0, 1)));

	// Populate arguments and compute the importance value
	*vis = Visibility(ref, lensIntr);
	*wi = lensIntr.p - ref.p;
	Float dist = wi->Length();
	*wi /= dist;

	// Compute PDF for importance arriving at _ref_

	// Compute lens area of perspective camera
	Float lensArea = lensRadius != 0 ? (Pi * lensRadius * lensRadius) : 1;
	*pdf = (dist * dist) / (AbsDot(lensIntr.n, *wi) * lensArea);
	return We(lensIntr.generate_ray(-*wi), pRaster);
}

}	//namespace valley