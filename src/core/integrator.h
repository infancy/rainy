#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_INTEGTATOR_H
#define VALLEY_CORE_INTEGTATOR_H

#include"valley.h"

namespace valley
{

class Intrgrator
{
public:
	virtual void render() = 0;	

private:
	//all render algorithms need to implement render func,but the diffence of details is quiet big 
	//so use the camera as a member of intrgrator
	std::unique_ptr<Camera> camera;
};

class RayCast : Intrgrator
{
public:
	void render();
};


}	//namespace valley


#endif //VALLEY_CORE_INTEGTATOR_H
