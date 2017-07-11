#include"sphere.h"
#include"sampling.h"

namespace valley
{

// Sphere Method Definitions
Bounds3f Sphere::object_bound() const 
{
	return Bounds3f(Point3f(-radius, -radius, -radius),
					Point3f(radius, radius, radius));
}

bool Sphere::intersect(const Ray &r, Float* tHit, SurfaceIsect* isect,
	bool testAlphaTexture) const
{
	static const Float kEpsilon = 0.001f;

	//phi from 0 to 2Pi,theta from 0 to Pi
	static const Float phiMax = 2 * Pi, thetaMax = Pi, thetaMin = 0; 

	// Transform _Ray_ to object space
	Ray ray = (*WorldToObject)(r);

	ray.d = Normalize(ray.d);	//需不需要归一化？

	Float t;
	Vector3f v = ray.o - Point3f(0, 0, 0);
	//求解t^2 + 2bt + c = 0;		因为direction是单位向量，所以a = d * d = 1
	Float b = ray.d * v;
	Float c = v * v - radius * radius;
	Float discr = b * b - c;

	if (discr > 0.0)
	{
		Float sqrte = sqrt(discr);
		Point3f pHit;

		if ((t = -b - sqrte) > kEpsilon && t < ray.tMax)
			pHit = ray(t);
		else if ((t = -b + sqrte) > kEpsilon && t < ray.tMax)
			pHit = ray(t);
		else
			return false;

		// Refine sphere intersection point
		pHit *= radius / Distance(pHit, Point3f(0, 0, 0));
		if (pHit.x == 0 && pHit.y == 0) pHit.x = 1e-5f * radius;
		Float phi = std::atan2(pHit.y, pHit.x);
		if (phi < 0) phi += 2 * Pi;

		//计算uv、dpdu、dpdv
		//dndu、dndv用于纹理抗锯齿

		// Find parametric representation of sphere hit
		Float u = phi / phiMax;
		Float theta = std::acos(Clamp(pHit.z / radius, -1, 1));
		Float v = (theta - thetaMin) / (thetaMax - thetaMin);

		// Compute sphere $\dpdu$ and $\dpdv$
		Float zRadius = std::sqrt(pHit.x * pHit.x + pHit.y * pHit.y);
		Float invZRadius = 1 / zRadius;
		Float cosPhi = pHit.x * invZRadius;
		Float sinPhi = pHit.y * invZRadius;

		Vector3f dpdu(-phiMax * pHit.y, phiMax * pHit.x, 0);
		Vector3f dpdv = (thetaMax - thetaMin) *
			Vector3f(pHit.z * cosPhi, pHit.z * sinPhi, -radius * std::sin(theta));

		// Compute sphere $\dndu$ and $\dndv$
		Vector3f d2Pduu = -phiMax * phiMax * Vector3f(pHit.x, pHit.y, 0);
		Vector3f d2Pduv =
			(thetaMax - thetaMin) * pHit.z * phiMax * Vector3f(-sinPhi, cosPhi, 0.);
		Vector3f d2Pdvv = -(thetaMax - thetaMin) * (thetaMax - thetaMin) *
			Vector3f(pHit.x, pHit.y, pHit.z);

		// Compute coefficients for fundamental forms
		Float E = Dot(dpdu, dpdu);
		Float F = Dot(dpdu, dpdv);
		Float G = Dot(dpdv, dpdv);
		Vector3f N = Normalize(Cross(dpdu, dpdv));
		Float e = Dot(N, d2Pduu);
		Float f = Dot(N, d2Pduv);
		Float g = Dot(N, d2Pdvv);

		// Compute $\dndu$ and $\dndv$ from fundamental form coefficients
		Float invEGF2 = 1 / (E * G - F * F);
		Normal3f dndu = Normal3f((f * F - e * G) * invEGF2 * dpdu +
			(e * F - f * E) * invEGF2 * dpdv);
		Normal3f dndv = Normal3f((g * F - f * G) * invEGF2 * dpdu +
			(f * F - g * E) * invEGF2 * dpdv);

		Vector3f pError = gamma(5) * Abs((Vector3f)pHit);

		*isect = (*ObjectToWorld)(SurfaceIsect(pHit, pError, Point2f(u, v),
			-ray.d, dpdu, dpdv, dndu, dndv, this));

		// Update _tHit_ for quadric intersection
		*tHit = (Float)t;	////记录相交距离
		return true;
	}
	else
		return false;
}

bool Sphere::intersectP(const Ray &r, bool testAlphaTexture) const 
{
	static const Float kEpsilon = 0.001f;

	// Transform _Ray_ to object space
	Ray ray = (*WorldToObject)(r);

	ray.d = Normalize(ray.d);	//需不需要归一化？

	Float t;
	Vector3f v = ray.o - Point3f(0, 0, 0);
	//求解t^2 + 2bt + c = 0;		因为direction是单位向量，所以a = d * d = 1
	Float b = ray.d * v;
	Float c = v * v - radius * radius;
	Float discr = b * b - c;
	Float e = sqrt(discr);

	if (discr < 0.f || (t = -b + e) < kEpsilon || (t = -b - e) > ray.tMax)
		return false;
	else
		return true;
}

Float Sphere::area() const { return 4 * Pi * radius * radius; }

Isect Sphere::sample(const Point2f &u, Float *pdf) const 
{
	Point3f pObj = Point3f(0, 0, 0) + radius * uniform_sample_sphere(u);
	Isect it;
	it.n = Normalize((*ObjectToWorld)(Normal3f(pObj.x, pObj.y, pObj.z)));
	if (reverseOrientation) it.n *= -1;
	// Reproject _pObj_ to sphere surface and compute _pObjError_
	pObj *= radius / Distance(pObj, Point3f(0, 0, 0));
	Vector3f pObjError = gamma(5) * Abs((Vector3f)pObj);
	it.p = (*ObjectToWorld)(pObj, pObjError, &it.pError);
	*pdf = 1 / area();
	return it;
}

Isect Sphere::sample(const Isect &ref, const Point2f &u,
	Float *pdf) const 
{
	Point3f pCenter = (*ObjectToWorld)(Point3f(0, 0, 0));

	// Sample uniformly on sphere if $\pt{}$ is inside it
	Point3f pOrigin =
		offset_ray_origin(ref.p, ref.pError, ref.n, pCenter - ref.p);
	if (DistanceSquared(pOrigin, pCenter) <= radius * radius) {
		Isect intr = sample(u, pdf);
		Vector3f wi = intr.p - ref.p;
		if (wi.LengthSquared() == 0)
			*pdf = 0;
		else {
			// Convert from area measure returned by Sample() call above to
			// solid angle measure.
			wi = Normalize(wi);
			*pdf *= DistanceSquared(ref.p, intr.p) / AbsDot(intr.n, -wi);
		}
		if (std::isinf(*pdf)) *pdf = 0.f;
		return intr;
	}

	// Compute coordinate system for sphere sampling
	Vector3f wc = Normalize(pCenter - ref.p);
	Vector3f wcX, wcY;
	CoordinateSystem(wc, &wcX, &wcY);

	// Sample sphere uniformly inside subtended cone

	// Compute $\theta$ and $\phi$ values for sample in cone
	Float sinThetaMax2 = radius * radius / DistanceSquared(ref.p, pCenter);
	Float cosThetaMax = std::sqrt(std::max((Float)0, 1 - sinThetaMax2));
	Float cosTheta = (1 - u[0]) + u[0] * cosThetaMax;
	Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
	Float phi = u[1] * 2 * Pi;

	// Compute angle $\alpha$ from center of sphere to sampled point on surface
	Float dc = Distance(ref.p, pCenter);
	Float ds = dc * cosTheta -
		std::sqrt(std::max(
		(Float)0, radius * radius - dc * dc * sinTheta * sinTheta));
	Float cosAlpha = (dc * dc + radius * radius - ds * ds) / (2 * dc * radius);
	Float sinAlpha = std::sqrt(std::max((Float)0, 1 - cosAlpha * cosAlpha));

	// Compute surface normal and sampled point on sphere
	Vector3f nWorld =
		SphericalDirection(sinAlpha, cosAlpha, phi, -wcX, -wcY, -wc);
	Point3f pWorld = pCenter + radius * Point3f(nWorld.x, nWorld.y, nWorld.z);

	// Return _Isect_ for sampled point on sphere
	Isect it;
	it.p = pWorld;
	it.pError = gamma(5) * Abs((Vector3f)pWorld);
	it.n = Normal3f(nWorld);
	if (reverseOrientation) it.n *= -1;

	// Uniform cone PDF.
	*pdf = 1 / (2 * Pi * (1 - cosThetaMax));

	return it;
}

Float Sphere::pdf(const Isect &ref, const Vector3f &wi) const 
{
	Point3f pCenter = (*ObjectToWorld)(Point3f(0, 0, 0));
	// Return uniform PDF if point is inside sphere
	Point3f pOrigin =
		offset_ray_origin(ref.p, ref.pError, ref.n, pCenter - ref.p);
	if (DistanceSquared(pOrigin, pCenter) <= radius * radius)
		return Shape::pdf(ref, wi);

	// Compute general sphere PDF
	Float sinThetaMax2 = radius * radius / DistanceSquared(ref.p, pCenter);
	Float cosThetaMax = std::sqrt(std::max((Float)0, 1 - sinThetaMax2));
	return uniform_cone_pdf(cosThetaMax);
}

Float Sphere::solid_angle(const Point3f &p, int nSamples) const 
{
	Point3f pCenter = (*ObjectToWorld)(Point3f(0, 0, 0));
	if (DistanceSquared(p, pCenter) <= radius * radius)
		return 4 * Pi;
	Float sinTheta2 = radius * radius / DistanceSquared(p, pCenter);
	Float cosTheta = std::sqrt(std::max((Float)0, 1 - sinTheta2));
	return (2 * Pi * (1 - cosTheta));
}

}	//namespace valley