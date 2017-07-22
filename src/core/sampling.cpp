#include"sampling.h"

namespace valley
{

Vector3f uniform_sample_sphere(const Point2f &u)
{
	Float y = 1 - 2 * u[0];
	Float r = std::sqrt(std::max((Float)0, (Float)1 - y * y));
	Float phi = 2 * Pi * u[1];
	return Vector3f(r * std::sin(phi), y, r * std::cos(phi));
}

Vector3f uniform_sample_hemisphere(const Point2f& u)
{
	Float y = u[0];
	Float r = std::sqrt(std::max((Float)0, (Float)1. - y * y));
	Float phi = 2 * Pi * u[1];
	return Vector3f(r * std::sin(phi), y, r * std::cos(phi));
}

Point2f concentric_sample_disk(const Point2f& u)
{
	// Map uniform random numbers to $[-1,1]^2$
	Point2f uOffset = 2.f * u - Vector2f(1, 1);

	// Handle degeneracy at the origin
	if (uOffset.x == 0 && uOffset.y == 0) return Point2f(0, 0);

	// Apply concentric mapping to point	
	Float theta, r;
	if (std::abs(uOffset.x) > std::abs(uOffset.y))
	{
		r = uOffset.x;
		theta = PiOver4 * (uOffset.y / uOffset.x);	// y/x from -1 to 1
	}
	else
	{
		r = uOffset.y;
		theta = PiOver2 - PiOver4 * (uOffset.x / uOffset.y);
	}
	return r * Point2f(std::cos(theta), std::sin(theta));	
}

}	//namespace valley