#include"Rectangle.h"
#include"sampling.h"

namespace valley
{

// Rectangle Method Definitions
Bounds3f Rectangle::object_bound() const
{
	return Bounds3f(point, point + up + right);
}

bool Rectangle::intersect(const Ray &r, SurfaceIsect* isect
	/*bool testAlphaTexture*/) const
{
	// Transform _Ray_ to object space
	Ray ray = (*WorldToObject)(r);

	ray.d = Normalize(ray.d);	//需不需要归一化？

	Vector3f vec = point - ray.o;
	const Normal3f& n = normal;
	//tHit = (point - ray.o) * normal / (ray.d * normal); 
	Float tmpa = (vec.x * n.x + vec.y * n.y + vec.z * n.z);
	Float tmpb = (ray.d.x * n.x + ray.d.y * n.y + ray.d.z * n.z);
	if (tmpb == 0.f)
		return false;		//平行
	Float t = tmpa / tmpb;

	if (t <= 0.f || t >= ray.tMax)
		return false;

	Point3f pHit = ray.o + t * ray.d;
	Vector3f d = pHit - point;

	Float dDotup = d * up;

	if (dDotup < 0.0 || dDotup > up * up)
		return false;

	Float dDotright = d * right;

	if (dDotright < 0.0 || dDotright > right * right)
		return false;

	//u、v from 0 to 1
	Float u = d * right / right.Length();
	Float v = d * up / up.Length();
	
	Vector3f dpdu(right.Length(), 0, 0), dpdv(0, up.Length(), 0);
	Normal3f dndu(0, 0, 0), dndv(0, 0, 0);

	Vector3f pError(0, 0, 0);

	*isect = (*ObjectToWorld)(SurfaceIsect(pHit, pError, Point2f(u, v),
		-ray.d, dpdu, dpdv, dndu, dndv, this));

	ray.tMax = t;	//记录相交距离
	return true;
}

bool Rectangle::intersectP(const Ray &r, bool testAlphaTexture) const
{
	// Transform _Ray_ to object space
	Ray ray = (*WorldToObject)(r);

	ray.d = Normalize(ray.d);	//需不需要归一化？

	Vector3f vec = point - ray.o;
	const Normal3f& n = normal;
	//tHit = (point - ray.o) * normal / (ray.d * normal); 
	Float tmpa = (vec.x * n.x + vec.y * n.y + vec.z * n.z);
	Float tmpb = (ray.d.x * n.x + ray.d.y * n.y + ray.d.z * n.z);
	if (tmpb == 0.f)
		return false;		//平行
	Float t = tmpa / tmpb;

	if (t <= 0.f)
		return false;

	Point3f pHit = ray.o + t * ray.d;
	Vector3f d = pHit - point;

	Float dDotup = d * up;

	if (dDotup < 0.0 || dDotup > up * up)
		return false;

	Float dDotright = d * right;

	if (dDotright < 0.0 || dDotright > right * right)
		return false;

	return true;
}

Float Rectangle::area() const { return up.Length() * right.Length(); }

Isect Rectangle::sample(const Point2f &u, Float *pdf) const
{
	Point3f pObj(u.x * right.Length(), u.y * up.Length(), 0);
	Isect it;
	it.n = Normalize((*ObjectToWorld)(Normal3f(0, 0, 1)));
	if (reverseOrientation) it.n *= -1;
	it.p = (*ObjectToWorld)(pObj, Vector3f(0, 0, 0), &it.pError);
	*pdf = 1 / area();
	return it;
}

}	//namespace valley