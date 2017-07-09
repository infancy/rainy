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

#ifndef VALLEY_CORE_BSDF_H
#define VALLEY_CORE_BSDF_H

#include"valley.h"
#include"geometry.h"
//#include"intersection.h"

namespace valley
{

//Auxiliary Function
inline Float AbsCosTheta(const Vector3f& w) { return std::abs(w.z); }

inline bool same_hemisphere(const Vector3f& w, const Vector3f& wp) { return w.z * wp.z > 0; }
inline bool same_hemisphere(const Vector3f& w, const Normal3f& wp) { return w.z * wp.z > 0; }


enum class BxDF_type 
{
	reflection   = 1 << 0,
	transmission = 1 << 1,
	diffuse		 = 1 << 2,
	glossy		 = 1 << 3,
	specular	 = 1 << 4,
	all			 = reflection | transmission | diffuse | glossy | specular
};

constexpr BxDF_type operator&(BxDF_type a, BxDF_type b)
{
	return static_cast<BxDF_type>(static_cast<int>(a) & static_cast<int>(b));
}
constexpr BxDF_type operator|(BxDF_type a, BxDF_type b)
{
	return static_cast<BxDF_type>(static_cast<int>(a) | static_cast<int>(b));
}

//某些光线传输算法需要对BRDF和BTDF进行区分，所以对BxDF加入type成员
class BxDF 
{
public:
	BxDF(BxDF_type t) : type(t) {}
	virtual ~BxDF() {}
	
	bool match(BxDF_type t) const { return (t & type) == type; }	

	//针对给定方向返回分布函数值
	virtual Color4f f(const Vector3f& wo, const Vector3f& wi) const = 0;
	//略复杂
	virtual Color4f sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& sample, 
							 Float* Pdf, BxDF_type* sampledType = nullptr) const;

	//rho_hemisphere_direction
	virtual Color4f rho(const Vector3f& wo, int nSamples,
						const Point2f* samples) const;
	//rho_hemisphere_hemisphere
	virtual Color4f rho(int nSamples, const Point2f* samples1,
						const Point2f* samples2) const;

	virtual Float pdf(const Vector3f& wo, const Vector3f& wi) const;
	//virtual std::string ToString() const = 0;

	const BxDF_type type;
};

class BSDF 
{
public:
	BSDF(const SurfaceIsect& si, Float eta = 1);
	~BSDF() {}				//使用系统的 new 和 delete

	void add_BxDF(BxDF* b);
	int components_num(BxDF_type flags = BxDF_type::all) const;

	Vector3f world_to_local(const Vector3f& v) const;
	Vector3f local_to_world(const Vector3f& v) const;

	//针对给定方向返回分布函数值
	Color4f f(const Vector3f& woW, const Vector3f& wiW,
			  BxDF_type flags = BxDF_type::all) const;
	Color4f sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, Float* pdf,
					 BxDF_type type = BxDF_type::all, BxDF_type* sampledType = nullptr) const;

	Color4f rho(int nSamples, const Point2f* samples1, const Point2f* samples2,
				BxDF_type flags = BxDF_type::all) const;
	Color4f rho(const Vector3f& wo, int nSamples, const Point2f* samples,
				BxDF_type flags = BxDF_type::all) const;

	Float pdf(const Vector3f& wo, const Vector3f& wi,
			  BxDF_type flags = BxDF_type::all) const;
	//std::string ToString() const;

public:
	const Float eta;	//表面相对折射率，用于（半）透明物体

private:
	const Normal3f ns, ng;	//shading-normal/geometry-normal
	const Vector3f ss, ts;	//ns,ss,ts构成了着色坐标系

	static constexpr int MaxBxDFs = 8;	//最大挂载8个BxDF组件
	int nBxDFs = 0;
	BxDF* bxdfs[MaxBxDFs];
	//friend class MixMaterial;
};

// BSDF Inline Method Definitions

inline void BSDF::add_BxDF(BxDF* b)
{
	CHECK_LT(nBxDFs, MaxBxDFs);
	bxdfs[nBxDFs++] = b;
}

inline Vector3f BSDF::world_to_local(const Vector3f &v) const
{
	return Vector3f(Dot(v, ss), Dot(v, ts), Dot(v, ns));
}
inline Vector3f BSDF::local_to_world(const Vector3f &v) const
{	
	return Vector3f(ss.x * v.x + ts.x * v.y + ns.x * v.z,	//因为stn是正交矩阵，其逆矩阵即为转置矩阵
					ss.y * v.x + ts.y * v.y + ns.y * v.z,
					ss.z * v.x + ts.z * v.y + ns.z * v.z);
}

}	//namespace valley


#endif //VALLEY_CORE_BSDF_H