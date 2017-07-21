#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_FILTER_H
#define VALLEY_CORE_FILTER_H

#include"valley.h"
#include"geometry.h"

namespace valley
{

class Filter 
{
public:
	Filter(const Vector2f &radius)
		: radius(radius), invRadius(Vector2f(1.f / radius.x, 1.f / radius.y)) {}
	virtual ~Filter() {}

	virtual Float evaluate(const Point2f &p) const = 0;

public:
	const Vector2f radius, invRadius;
};


class BoxFilter : public Filter 
{
public:
	BoxFilter(const Vector2f& radius = Vector2f(0.5, 0.5)) : Filter(radius) {}

	Float evaluate(const Point2f &p) const { return 1.0; }
};


class GaussianFilter : public Filter 
{
public:
	GaussianFilter(const Vector2f& radius = Vector2f(1.0, 1.0), Float alpha = 1.0) :
		Filter(radius), alpha(alpha),
		expX(std::exp(-alpha * radius.x * radius.x)),
		expY(std::exp(-alpha * radius.y * radius.y)) {}

	Float evaluate(const Point2f &p) const
	{
		return gaussian(p.x, expX) * gaussian(p.y, expY);
	}

private:
	Float gaussian(Float d, Float expv) const
	{
		return std::max((Float)0, Float(std::exp(-alpha * d * d) - expv));
	}

private:
	const Float alpha;
	const Float expX, expY;
};


class LanczosSincFilter : public Filter 
{
public:
	LanczosSincFilter(const Vector2f& radius = Vector2f(2.0, 2.0), Float tau = 1.5)
		: Filter(radius), tau(tau) {}

	Float evaluate(const Point2f &p) const
	{
		return windowed_sinc(p.x, radius.x) * windowed_sinc(p.y, radius.y);
	}

private:
	Float sinc(Float x) const
	{
		x = std::abs(x);
		if (x < 1e-5) return 1;
		return std::sin(Pi * x) / (Pi * x);
	}

	Float windowed_sinc(Float x, Float radius) const
	{
		x = std::abs(x);
		if (x > radius) return 0;
		Float lanczos = sinc(x / tau);
		return sinc(x) * lanczos;
	}

private:
	const Float tau;
};

}	//namespace valley


#endif //VALLEY_CORE_FILTER_H

