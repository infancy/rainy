#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef VALLEY_CORE_SAMPLER_H
#define VALLEY_CORE_SAMPLER_H

#include"valley.h"
#include"rng.h"
#include"camera.h"

namespace valley
{

class Sampler
{
public:
	Sampler(int64_t samples_PerPixel): 
		samples_PerPixel(samples_PerPixel) {}
	~Sampler(){}

	virtual Float   get_1D() = 0;
	virtual Point2f get_2D() = 0;

	CameraSample get_CameraSample(int x, int y);
	CameraSample get_CameraSample(Point2i p);

	//调用 start_pixel 生成一个像素中所有的样本
	virtual void start_pixel(const Point2i& p);
	virtual bool next_sample();
	virtual bool set_SampleIndex(int64_t sampleNum);

	virtual std::unique_ptr<Sampler> clone(int seed) = 0;

public:
	const int64_t samples_PerPixel;

protected:
	Point2i currentPixel;	//当前在哪个像素上
	int64_t currentPixel_SampleIndex;	//在该像素的第几个采样点上

	/*
	//每个子数组的大小为 n * samples_PerPixel，subArraySizes 记录的是这个 n 的大小
	vector<int> subArraySizes_1D, subArraySizes_2D;	
	vector<vector<Float>>   sampleArray1D;
	vector<vector<Point2f>> sampleArray2D;

	size_t current_ArrayOffset_1D, current_ArrayOffset_2D;
	
	const Point2f *Sampler::Get2DArray(int n) {
		...
		if( subArraySizes_2D[current_ArrayOffset_2D] == n && currentPixel_SampleIndex < samples_PerPixel)
			return &sampleArray2D[current_ArrayOffset_2D++][currentPixel_SampleIndex * n];}
	*/
};

//采样时以某个像素为采样空间，提前生成采样数组，而后调用该数组中的样本
class PixelSampler : public Sampler
{
public:
	//默认准备 83 组样本
	PixelSampler(int64_t samplesPerPixel, int seed = 1234, int nSampledDimensions = 83);
	~PixelSampler() {}

	virtual Float   get_1D() override;
	virtual Point2f get_2D() override;

	//void start_pixel(const Point2i& p) 由子类实现，在其中完成初始化采样数组的工作
	bool next_sample();
	bool set_SampleIndex(int64_t sampleNum);

protected:
	RNG rng;

	std::vector<std::vector<Float>>   sampleArray_1D;
	std::vector<std::vector<Point2f>> sampleArray_2D;
	int current_ArrayOffset_1D = 0, current_ArrayOffset_2D = 0;	  //当前的数组下标
};

//采样时以整个 Film 为采样空间
class GlobalSampler : public Sampler 
{
public:
	GlobalSampler(int64_t samplesPerPixel) : Sampler(samplesPerPixel) {}
	~GlobalSampler() {}

	virtual Float   get_1D() override;
	virtual Point2f get_2D() override;

	//virtual void start_pixel(const Point2i& p);
	virtual bool next_sample();
	virtual bool set_SampleIndex(int64_t sampleNum);
	
	virtual int64_t get_index_for_sample(int64_t sampleNum) const = 0;
	//在某一维度上进行采样
	virtual Float sample_dimension(int64_t index, int dimension) const = 0;

private:
	int dimension;
	int64_t intervalSampleIndex;

	static const int arrayStartDim = 5;
	int arrayEndDim;
};

}	//namespace valley


#endif //VALLEY_CORE_SAMPLER_H

