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

Vector3f uniform_sample_hemisphere(const Point2f& u);
Float    uniform_hemisphere_pdf();

Point2f ConcentricSampleDisk(const Point2f &u);

inline Vector3f cosine_sample_hemisphere(const Point2f &u)
{
	Point2f d = ConcentricSampleDisk(u);
	Float z = std::sqrt(std::max((Float)0, 1 - d.x * d.x - d.y * d.y));
	return Vector3f(d.x, d.y, z);
}
inline Float cosine_hemisphere_pdf(Float cosTheta) { return cosTheta * InvPi; }

}	//namespace valley


#endif //VALLEY_CORE_SAMPLING_H
