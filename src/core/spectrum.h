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
//所有检查都在
class Spectrum
{
public:
	Spectrum(float f = 0.f) : r(f), g(f), b(f), a(1.f) { DCHECK(!isnan()); }
	Spectrum(float r, float g, float b, float a = 1.f)
		: r(r), g(g), b(b), a(a) { DCHECK(!isnan()); }
	Spectrum(const Spectrum& c) : r(c.r), g(c.g), b(c.b), a(c.a) { DCHECK(!isnan()); }
	Spectrum operator=(const Spectrum& c) {
		if (this != &c) {
			DCHECK(!c.isnan());
			r = c.r; g = c.g; b = c.b; a = c.a; }
		return *this;}
	~Spectrum(){}

	Spectrum& operator=(float x) { r = g = b = x; return *this; }

	Spectrum operator*(const float f) const { return Spectrum(r * f, g * f, b * f, a * f); }
	Spectrum operator/(const float f) const { return operator*(1.f / f); }

	Spectrum& operator*=(const float f) { r *= f; g *= f; b *= f; a *= f; return *this; }
	Spectrum& operator/=(const float f) { return operator*=(1.f / f); }

	bool operator==(const Spectrum& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
	bool operator!=(const Spectrum& c) const { return !operator==(c); }

	Spectrum operator+(const Spectrum& c) const { return Spectrum(r + c.r, g + c.g, b + c.b, a + c.a); }
	Spectrum operator-(const Spectrum& c) const { return Spectrum(r - c.r, g - c.g, b - c.b, a - c.a); }
	Spectrum operator*(const Spectrum& c) const { return Spectrum(r * c.r, g * c.g, b * c.b, a * c.a); }
	Spectrum operator/(const Spectrum& c) const { return Spectrum(r / c.r, g / c.g, b / c.b, a / c.a); }

	Spectrum& operator+=(const Spectrum& c) { r += c.r; g += c.g; b += c.b; a += c.a; return *this; }
	Spectrum& operator-=(const Spectrum& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
	Spectrum& operator*=(const Spectrum& c) { r *= c.r, g *= c.g, b *= c.b, a *= c.a; return *this; }

	bool isnan() const {
		return std::isnan(r) || std::isnan(g) || std::isnan(b) || std::isnan(a);
	}

	const Spectrum& operator[](int i) const
	{
		DCHECK(i == 0 || i == 1 || i == 2);
		return (i == 0) ? r : ((i == 1) ? g : b);
	}

	bool is_black() const { return r == 0.f && g == 0.f && b == 0.f; }
	Float max_value() const { return std::max(std::max(r, g), b); }
	float luminance() const { return r * 0.212671f + g * 0.715160f + b * 0.072169f; }


	Spectrum normalize() const
	{
		float length = sqrtf(r * r + g * g + b * b);
		float factor = 0.f;
		if (length > 0.f)
			factor = 1.f / length;
		return Spectrum((r * factor), (g * factor), (b * factor), a);
	}

	Spectrum clamp(Float low = 0, Float high = Infinity) const
	{
		Spectrum ret;
		
		ret.r = Clamp(r, low, high);
		ret.g = Clamp(g, low, high);
		ret.b = Clamp(b, low, high);

		DCHECK(!ret.isnan());
		return ret;
	}

	Spectrum sqrt()
	{
		return Spectrum(std::sqrt(r), std::sqrt(g), std::sqrt(b), std::sqrt(a));
	}
	/*
	friend Spectrum Sqrt(const Spectrum& s)
	{
		Spectrum ret;
		for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] = std::sqrt(s.c[i]);
		DCHECK(!ret.HasNaNs());
		return ret;
	}
	*/
	/*
	Spectrum lerp(Spectrum& c, float f) { return Spectrum((*this * f) + (c * (1 - f))); }
	*/

public:
	float r, g, b, a;
};
/*
template <typename T, typename U>
inline Spectrum operator*(U s, const Spectrum& v) 
{
	return v * s;
}
*/
inline Spectrum operator*(const float f, const Spectrum& c) { return c.operator*(f); }

//template <typename T>
inline std::ostream &operator<<(std::ostream &os, const Spectrum& c) 
{
	os.precision(4);	//两位精度
	os << "[ " << c.r << ", " << c.g << ", " << c.b << " ]";
	return os;
}

}	//namespace valley


#endif //VALLEY_CORE_COLOR_H
