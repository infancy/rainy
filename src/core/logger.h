#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CORE_LOGGER_H
#define RAINY_CORE_LOGGER_H

#include"rainy.h"

namespace rainy
{

#ifdef _DEBUG
#define Log(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)  
#else  
#define Log(format,...)  
#endif  

}	//namespace rainy


#endif //RAINY_CORE_LOGGER_H
