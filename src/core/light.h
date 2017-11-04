/*

pbrt source code is Copyright(c) 1998-2016
Matt Pharr, Greg Humphreys, and Wenzel Jakob.

This file is part of pbrt.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_LIGHT_H
#define RAINY_CORE_LIGHT_H

#include"rainy.h"
#include"transform.h"
#include"interaction.h"
#include"spectrum.h"

namespace rainy
{

enum class LightType
{
	DeltaPosition  = 1,
	DeltaDirection = 2,
	Area		   = 4,
	Infinite       = 8
};

inline bool is_DeltaLight(int flags) 
{
	return flags & static_cast<int>(LightType::DeltaPosition) ||
		   flags & static_cast<int>(LightType::DeltaDirection);
}

class Light
{
public:
	Light(
		int flags,				//在蒙特卡洛算法中需要区分光源的类型
		const Transform& LightToWorld, 
	  //const MediumInterface &mediumInterface,
		int nSamples = 1);		//用于区域光源，发射阴影光线，生成软阴影
	virtual ~Light();

	//总发射功率，只需要计算近似值即可
	virtual Spectrum power() const = 0;

	//在开始渲染前记录场景的一些特征，如DistanceLight记录场景包围盒
	virtual void preprocess(const Scene &scene) {}

	//光源向-r方向发射的辐射度
	//比如当视点发出的光线与场景中物体未相交时，计算光源朝视点方向的辐射度
	virtual Spectrum Le(const RayDifferential &r) const;

	// 从 ref->wi->light 的概率
	virtual Float pdf_Li(const Interaction& ref, const Vector3f &wi) const = 0;
	// 传入 ray 和 light_normal，计算 pdfPos，pdfDir
	virtual void  pdf_Le(const Ray &ray, const Normal3f &nLight, Float *pdfPos,
						 Float *pdfDir) const = 0;

	// 传入交点，在光源上选取一点，计算选到该点的概率密度，该点到交点的方向 wi、入射辐射度（incident Radiance） Li 及可见性
	// 当光源是面积光源时，还需传入一个[0,1]^2范围的采样点，对面积光源上一点进行采样并记录相应**概率密度值pdf**
	virtual Spectrum sample_Li(const Interaction& ref, const Point2f& u,
							Vector3f* wi, Float* pdf, Visibility* vis) const = 0;

	//pdf of position、pdf of direction
	//在光源上取一点 pos，在 pos 上取一个方向 dir，生成 ray
	virtual Spectrum sample_Le(const Point2f& u1, const Point2f& u2,  Ray* ray, 
							Normal3f* nLight, Float* pdfPos, Float* pdfDir) const = 0;

public:
	const int flags;
	const int nSamples;
	//const MediumInterface mi;

protected:
	const Transform LightToWorld, WorldToLight;
};


class Visibility
{
public:
	Visibility() {}
	// VisibilityTester Public Methods
	Visibility(const Interaction& p0, const Interaction& p1)
		: p0(p0), p1(p1) {}

	// 某些光线传输方法需要这两个点
	const Interaction& P0() const { return p0; }
	const Interaction& P1() const { return p1; }

	bool unoccluded(const Scene &scene) const;

	// transmittance
	// 处理当两点在介质中时的辐射度
	Spectrum Tr(const Scene &scene, Sampler &sampler) const;

private:
	// 交点、光源，用于构造阴影光线
	Interaction p0, p1;
};

class AreaLight : public Light 
{
public:
	// AreaLight Interface
	AreaLight(const Transform& LightToWorld, 
	  //const MediumInterface &medium,
		int nSamples);

	//根据光源表面一点与表面法线计算出射方向上的发射辐射度L
	virtual Spectrum L(const Interaction& intr, const Vector3f& w) const = 0;
};

}	//namespace rainy


#endif //RAINY_CORE_LIGHT_H
