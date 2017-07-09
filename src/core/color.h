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
	float r, g, b, a;

public:
	Color(float f = 0.f) : r(f), g(f), b(f), a(1.f) { DCHECK(!hasNaNs()); }
	Color(float r, float g, float b, float a = 1.f)
		: r(r), g(g), b(b), a(a) { DCHECK(!hasNaNs()); }
	Color(const Color& c) : r(c.r), g(c.g), b(c.b), a(c.a) { DCHECK(!hasNaNs()); }
	Color operator=(const Color& c) {
		if (this != &c) {
			DCHECK(!c.hasNaNs());
			r = c.r; g = c.g; b = c.b; a = c.a; }
		return *this;}
	~Color(){}

	Color& operator=(float x) { r = g = b = x; return *this; }

	bool operator==(const Color& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
	bool operator!=(const Color& c) const { return !operator==(c); }

	Color operator+(const Color& c) const { return Color(r + c.r, g + c.g, b + c.b, a + c.a); }
	Color operator-(const Color& c) const { return Color(r - c.r, g - c.g, b - c.b, a - c.a); }

	Color operator*(const float f) const { return Color(r * f, g * f, b * f, a * f); }
	Color operator/(const float f) const { return operator*(1.f / f); }

	Color& operator+=(const Color& c) { r += c.r; g += c.g; b += c.b; a += c.a; return *this; }
	Color& operator-=(const Color& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
	Color& operator*=(const float f) { r *= f; g *= f; b *= f; a *= f; return *this; }
	Color& operator/=(const float f) { return operator*=(1.f / f); }

	bool is_black() const { return r == 0.f && g == 0.f && b == 0.f; }

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

		DCHECK(!ret.hasNaNs());
		return ret;
	}

	/*
	Color Mix(Color& c)
	{
		return Color(r * c.r, g * c.g, b * c.b);
	}

	Color mix_f(float f, Color& c)
	{
		return ((*this * f) + (c * (1 - f)));
	}

	inline Color saturate()
	{

		r = max(min(r, (T)255), (T)0);
		g = max(min(g, (T)255), (T)0);
		b = max(min(b, (T)255), (T)0);
		return Color(r, g, b);
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
