#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_COLOR_H
#define VALLEY_CORE_COLOR_H

#include"valley.h"
//#include"geometry.h"

namespace valley
{

class Color4f
{
public:
	float r, g, b, a;

public:
	Color4f(float f = 0.f) : r(f), g(f), b(f), a(1.f) { DCHECK(!hasNaNs()); }
	Color4f(float r, float g, float b, float a = 1.f)
		: r(r), g(g), b(b), a(a) { DCHECK(!hasNaNs()); }
	Color4f(const Color4f& c) : r(c.r), g(c.g), b(c.b), a(c.a) { DCHECK(!hasNaNs()); }
	Color4f operator=(const Color4f& c) {
		if (this != &c) {
			DCHECK(!c.hasNaNs());
			r = c.r; g = c.g; b = c.b; a = c.a; }
		return *this;}
	~Color4f(){}

	Color4f& operator=(float x) { r = g = b = x; return *this; }

	bool operator==(const Color4f& c) { return r == c.r && g == c.g && b == c.b && a == c.a; }
	bool operator!=(const Color4f& c) { return !operator==(c); }

	Color4f operator+(Color4f& c) { return Color4f(r + c.r, g + c.g, b + c.b, a + c.a); }
	Color4f operator-(Color4f& c) { return Color4f(r - c.r, g - c.g, b - c.b, a - c.a); }

	Color4f operator*(float f) { return Color4f(r * f, g * f, b * f, a * f); }
	Color4f operator/(float f) { return operator*(1.f / f); }

	Color4f& operator+=(const Color4f& c) { r += c.r; g += c.g; b += c.b; a += c.a; return *this; }
	Color4f& operator-=(const Color4f& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
	Color4f& operator*=(const float f) { r *= f; g *= f; b *= f; a *= f; return *this; }
	Color4f& operator/=(const float f) { return operator*=(1.f / f); }

	bool is_black() { return r == 0.f && g == 0.f && b == 0.f; }

	Color4f normalize() const
	{
		float length = sqrtf(r * r + g * g + b * b);
		float factor = 0.f;
		if (length > 0.f)
			factor = 1.f / length;
		return Color4f((r * factor), (g * factor), (b * factor), a);
	}

	/*
	Color4f Mix(Color4f& c)
	{
		return Color4f(r * c.r, g * c.g, b * c.b);
	}

	Color4f mix_f(float f, Color4f& c)
	{
		return ((*this * f) + (c * (1 - f)));
	}
	
	inline Color4f saturate()
	{

		r = max(min(r, (T)255), (T)0);
		g = max(min(g, (T)255), (T)0);
		b = max(min(b, (T)255), (T)0);
		return Color4f(r, g, b);
	}
	*/
private:
	bool hasNaNs() const {
		//return isNaN(r) || isNaN(g) || isNaN(b) || isNaN(a);
		return std::isnan(r) || std::isnan(g) || std::isnan(b) || std::isnan(a);
	}
};

}	//namespace valley


#endif //VALLEY_CORE_COLOR_H
