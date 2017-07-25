#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_TEXTURE_H
#define VALLEY_CORE_TEXTURE_H

#include"valley.h"
#include"geometry.h"
#include"spectrum.h"

namespace valley
{

template <typename T>
class Texture 
{
public:
	//返回模板类型
	virtual T evaluate(const SurfaceInteraction& si) const = 0;
	virtual ~Texture() {}
};

template <typename T>
class ConstantTexture : public Texture<T> {
public:
	// ConstantTexture Public Methods
	ConstantTexture(const T& value) : value(value) {}
	T evaluate(const SurfaceInteraction&) const { return value; }

private:
	T value;
};

//using constant = Texture<Float>;
//using Spectrum4 = Texture<Spectrum>;

}	//namespace valley


#endif //VALLEY_CORE_TEXTURE_H
