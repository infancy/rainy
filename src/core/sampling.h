#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_SAMPLING_H
#define VALLEY_CORE_SAMPLING_H

#include"valley.h"
#include"geometry.h"

namespace valley
{

Vector3f uniform_sample_sphere(const Point2f &u);
Vector3f uniform_sample_hemisphere(const Point2f& u);

inline Float uniform_sphere_pdf()     { return Inv4Pi; }
inline Float uniform_hemisphere_pdf() { return Inv2Pi; }
inline Float uniform_cone_pdf(Float thetaMax) { return 1 / (2 * Pi * (1 - thetaMax)); }

//将[0,1]^2上的采样点映射到disk上
Point2f concentric_sample_disk(const Point2f &u);

inline Vector3f cosine_sample_hemisphere(const Point2f &u)
{
	Point2f d = concentric_sample_disk(u);
	Float z = std::sqrt(std::max((Float)0, 1 - d.x * d.x - d.y * d.y));
	return Vector3f(d.x, d.y, z);
}
inline Float    cosine_hemisphere_pdf(Float cosTheta) { return cosTheta * InvPi; }

}	//namespace valley


#endif //VALLEY_CORE_SAMPLING_H
