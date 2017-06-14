#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_INTERSECTION_H
#define VALLEY_CORE_INTERSECTION_H

#include"valley.h"
#include"geometry.h"

namespace valley
{

//Intersection 
struct Isect
{
	Isect(Float MaxDist = Infinity) : dist(MaxDist) {}

	Float dist;    //the distance to the closest intersection
	Point3f p;  //the intersection
	Vector3f pError;    //累积的浮点数绝对误差
	Vector3f wo;    //入射方向  
	Normal3f normal;     

	/*
	const Shape *shape = nullptr;   //所处的shape
    const Primitive *primitive = nullptr;
    BSDF *bsdf = nullptr;
	*/
};

//SurfaceIsct

}	//namespace valley


#endif //VALLEY_CORE_INTERSECTION_H
