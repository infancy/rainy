#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_COLOR_H
#define VALLEY_CORE_COLOR_H

#include"valley.h"
#include"pbrt.h"
//#include"geometry.h"

namespace valley
{

class Color
{
public:
	Color(float f = 0.f) : r(f), g(f), b(f), a(1.f) { DCHECK(!isnan()); }
	Color(float r, float g, float b, float a = 1.f)
		: r(r), g(g), b(b), a(a) { DCHECK(!isnan()); }
	Color(const Color& c) : r(c.r), g(c.g), b(c.b), a(c.a) { DCHECK(!isnan()); }
	Color operator=(const Color& c) {
		if (this != &c) {
			DCHECK(!c.isnan());
			r = c.r; g = c.g; b = c.b; a = c.a; }
		return *this;}
	~Color(){}

	Color& operator=(float x) { r = g = b = x; return *this; }

	Color operator*(const float f) const { return Color(r * f, g * f, b * f, a * f); }
	Color operator/(const float f) const { return operator*(1.f / f); }

	Color& operator*=(const float f) { r *= f; g *= f; b *= f; a *= f; return *this; }
	Color& operator/=(const float f) { return operator*=(1.f / f); }

	bool operator==(const Color& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
	bool operator!=(const Color& c) const { return !operator==(c); }

	Color operator+(const Color& c) const { return Color(r + c.r, g + c.g, b + c.b, a + c.a); }
	Color operator-(const Color& c) const { return Color(r - c.r, g - c.g, b - c.b, a - c.a); }
	Color operator*(const Color& c) const { return Color(r * c.r, g * c.g, b * c.b, a * c.a); }

	Color& operator+=(const Color& c) { r += c.r; g += c.g; b += c.b; a += c.a; return *this; }
	Color& operator-=(const Color& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
	Color& operator*=(const Color& c) { r *= c.r, g *= c.g, b *= c.b, a *= c.a; return *this; }

	bool isnan() const {
		return std::isnan(r) || std::isnan(g) || std::isnan(b) || std::isnan(a);
	}

	bool is_black() const { return r == 0.f && g == 0.f && b == 0.f; }
	Float max_value() const { return std::max(std::max(r, g), b); }
	float luminance() const { return r * 0.212671f + g * 0.715160f + b * 0.072169f; }


	Color normalize() const
	{
		float length = sqrtf(r * r + g * g + b * b);
		float factor = 0.f;
		if (length > 0.f)
			factor = 1.f / length;
		return Color((r * factor), (g * factor), (b * factor), a);
	}

	Color clamp(Float low = 0, Float high = Infinity) const
	{
		Color ret;
		
		ret.r = Clamp(r, low, high);
		ret.g = Clamp(g, low, high);
		ret.b = Clamp(b, low, high);

		DCHECK(!ret.isnan());
		return ret;
	}

	/*
	Color lerp(Color& c, float f) { return Color((*this * f) + (c * (1 - f))); }
	*/

public:
	float r, g, b, a;
};

//template <typename T>
inline std::ostream &operator<<(std::ostream &os, const Color& c) {
	os << "[ " << c.r << ", " << c.g << c.b << " ]";
	return os;
}

}	//namespace valley


#endif //VALLEY_CORE_COLOR_H
