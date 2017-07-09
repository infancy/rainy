#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_TEXTURE_H
#define VALLEY_CORE_TEXTURE_H

#include"valley.h"
#include"geometry.h"
#include"color.h"

namespace valley
{

template <typename T>
class Texture 
{
public:
	//返回模板类型
	virtual T evaluate(const SurfaceIsect& si) const = 0;
	virtual ~Texture() {}
};

template <typename T>
class ConstantTexture : public Texture<T> {
public:
	// ConstantTexture Public Methods
	ConstantTexture(const T &value) : value(value) {}
	T evaluate(const SurfaceIsect&) const { return value; }

private:
	T value;
};

//using constant = Texture<Float>;
//using Color4 = Texture<Color4f>;

}	//namespace valley


#endif //VALLEY_CORE_TEXTURE_H
