#include"Rectangle.h"
#include"sampling.h"

namespace rainy
{

// Rectangle Method Definitions
Bounds3f Rectangle::object_bound() const
{
	return Bounds3f(point, point + first + second);
}

bool Rectangle::intersect(const Ray &r, SurfaceInteraction* isect
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
	//CHECK(!(tmpa == 0 && tmpb == 0)) //在矩形上发射的平行于矩形的光线
	if (tmpb == 0.f)
		return false;		//平行
	Float t = tmpa / tmpb;

	if (t <= 0.f || t >= ray.tMax)
		return false;

	Point3f pHit = ray.o + t * ray.d;
	Vector3f d = pHit - point;

	Float dDotfirst = d * first;

	if (dDotfirst < 0.0 || dDotfirst > first * first)
		return false;

	Float dDotsecond = d * second;

	if (dDotsecond < 0.0 || dDotsecond > second * second)
		return false;

	//u、v from 0 to 1
	Float u = d * first / first.Length();
	Float v = d * second / second.Length();
	
	Vector3f dpdu(first.Length(), 0, 0), dpdv(0, second.Length(), 0);
	Normal3f dndu(0, 0, 0), dndv(0, 0, 0);

	Vector3f pError(0, 0, 0);

	//新建一个SI赋值给isect
	*isect = (*ObjectToWorld)(SurfaceInteraction(pHit, pError, Point2f(u, v),
		-ray.d, dpdu, dpdv, dndu, dndv, this));

	r.tMax = t;	//记录相交距离
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

	if (t <= 0.f || t >= ray.tMax)
		return false;

	Point3f pHit = ray.o + t * ray.d;
	Vector3f d = pHit - point;

	Float dDotfirst = d * first;

	if (dDotfirst < 0.0 || dDotfirst > first * first)
		return false;

	Float dDotsecond = d * second;

	if (dDotsecond < 0.0 || dDotsecond > second * second)
		return false;

	return true;
}

Float Rectangle::area() const { return first.Length() * second.Length(); }

//传入一个[0,1]^2范围内的随机点,返回一个交点和相应的pdf
Interaction Rectangle::sample(const Point2f &u, Float *pdf) const
{
	//Point3f pObj(u.x * first.Length(), u.y * second.Length(), 0);
	Point3f pObj = Point3f(u.x * first.Length(), u.y * second.Length(), 0) + point;
	Interaction it;
	//it.n = Normalize((*ObjectToWorld)(Normal3f(0, 0, 1)));
	it.n = Normalize((*ObjectToWorld)(Normal3f(0, 0, 1)));
	if (reverseOrientation) it.n *= -1;
	it.p = (*ObjectToWorld)(pObj, Vector3f(0, 0, 0), &it.pError);
	*pdf = 1 / area();
	return it;
}

}	//namespace rainy
