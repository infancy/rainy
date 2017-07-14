#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_LOGGER_H
#define VALLEY_CORE_LOGGER_H

#include"valley.h"

namespace valley
{

#ifdef _DEBUG
#define Log(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)  
#else  
#define Log(format,...)  
#endif  

}	//namespace valley


#endif //VALLEY_CORE_LOGGER_H
