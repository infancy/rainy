/*
pbrt source code is Copyright(c) 1998-2016
Matt Pharr, Greg Humphreys, and Wenzel Jakob.

This file is part of pbrt.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include"shape.h"
#include"transform.h"

namespace valley
{

// Shape Method Definitions
Shape::~Shape() {}

Shape::Shape(const Transform* ObjectToWorld, const Transform* WorldToObject,
	bool reverseOrientation)
	: ObjectToWorld(ObjectToWorld),
	WorldToObject(WorldToObject),
	reverseOrientation(reverseOrientation),
	transformSwapsHandedness(ObjectToWorld->SwapsHandedness()) {
	//++nShapesCreated;
}

Bounds3f Shape::world_bound() const { return (*ObjectToWorld)(object_bound()); }

bool Shape::intersectP(const Ray &ray,
	bool testAlphaTexture) const 
{
	return intersect(ray, nullptr /*testAlphaTexture*/);
}

//previous   next
//    ----  ----
//      \    /
//	  wo \  / wi
//		  \/
//		------
//		isect
Isect Shape::sample(const Isect &isect, const Point2f &u,
	Float *pdf) const 
{
	Isect next = sample(u, pdf);
	Vector3f wi = next.p - isect.p;
	if (wi.LengthSquared() == 0)
		*pdf = 0;
	else 
	{
		wi = Normalize(wi);
		// Convert from area measure, as returned by the Sample() call
		// above, to solid angle measure.
		Float dist = DistanceSquared(isect.p, next.p);
		Float cosine = AbsDot(next.n, -wi);
		*pdf *= dist / cosine;
		//DLOG(INFO) << "isect.p: " << isect.p << " next.p: " << next.p << " DistSquared: " << dist;
		//DLOG(INFO) << "next.n: " << next.n << " -wi: " << -wi << " AbsDot(intr.n, -wi):" << cosine;
		//DLOG(INFO) << "lightPdf: " << *pdf;
		if (std::isinf(*pdf)) *pdf = 0.f;
	}
	return next;
}

Float Shape::pdf(const Isect &ref, const Vector3f &wi) const 
{
	// Intersect sample ray with area light geometry
	Ray ray = ref.generate_ray(wi);
	SurfaceIsect isectLight;
	// Ignore any alpha textures used for trimming the shape when performing
	// this intersection. Hack for the "San Miguel" scene, where this is used
	// to make an invisible area light.
	if (!intersect(ray, &isectLight/*false*/)) return 0;

	// Convert light sample weight to solid angle measure
	Float pdf = DistanceSquared(ref.p, isectLight.p) /
		(AbsDot(isectLight.n, -wi) * area());
	if (std::isinf(pdf)) pdf = 0.f;
	return pdf;
}

Float Shape::solid_angle(const Point3f &p, int nSamples) const
{
	//Isect ref(p, Normal3f(), Vector3f(), Vector3f(0, 0, 1));
	Isect ref(p, Normal3f(), Vector3f(), Vector3f(0, 1, 0));
	double solidAngle = 0;

	for (int i = 0; i < nSamples; ++i) 
	{
		//Point2f u{ RadicalInverse(0, i), RadicalInverse(1, i) };
		Point2f u{0.f, 1.f};
		Float pdf;
		Isect pShape = sample(ref, u, &pdf);
		if (pdf > 0 && !intersectP(Ray(p, pShape.p - p, .999f))) 
		{
			solidAngle += 1 / pdf;
		}
	}
	return solidAngle / nSamples;
}

}	//namespace valley