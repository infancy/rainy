#include"sampler.h"

namespace valley
{

//
// Sampler Method Definitions
//

CameraSample Sampler::get_CameraSample(int x, int y)
{
	CameraSample cs;
	cs.pFilm = Point2f(x, y) + get_2D();
	cs.pLens = get_2D();
	return cs;
}

CameraSample Sampler::get_CameraSample(Point2i p)
{
	CameraSample cs;
	cs.pFilm = (Point2f)p + get_2D();
	cs.pLens = get_2D();
	return cs;
}

void Sampler::start_pixel(const Point2i& p)
{
	currentPixel = p;
	currentPixel_SampleIndex = 0;
}

bool Sampler::next_sample()
{
	// 若以完成当前像素的全部采样点则停止采样
	return ++currentPixel_SampleIndex < samples_PerPixel;
}

bool Sampler::set_SampleIndex(int64_t sampleNum)
{
	currentPixel_SampleIndex = sampleNum;
	return currentPixel_SampleIndex < samples_PerPixel;
}

//
// PixelSampler Method Definitions
//

PixelSampler::PixelSampler(int64_t samples_PerPixel, int seed, int nSampledDimensions)
	: Sampler(samples_PerPixel), rng(seed)
{
	for (int i = 0; i < nSampledDimensions; ++i)
	{
		sampleArray_1D.push_back(std::vector<Float>(samples_PerPixel));
		sampleArray_2D.push_back(std::vector<Point2f>(samples_PerPixel));
	}
}

Float PixelSampler::get_1D()
{
	CHECK_LT(currentPixel_SampleIndex, samples_PerPixel);
	if (current_ArrayOffset_1D < sampleArray_1D.size())
		return sampleArray_1D[current_ArrayOffset_1D++][currentPixel_SampleIndex];
	else
		return rng.get_1D();
}

Point2f PixelSampler::get_2D()
{
	CHECK_LT(currentPixel_SampleIndex, samples_PerPixel);
	if (current_ArrayOffset_2D < sampleArray_2D.size())
		return sampleArray_2D[current_ArrayOffset_2D++][currentPixel_SampleIndex];
	else
		return rng.get_2D();
}

bool PixelSampler::next_sample()
{
	current_ArrayOffset_1D = current_ArrayOffset_2D = 0;
	return Sampler::next_sample();
}

bool PixelSampler::set_SampleIndex(int64_t sampleNum)
{
	current_ArrayOffset_1D = current_ArrayOffset_2D = 0;
	return Sampler::set_SampleIndex(sampleNum);
}

//
// GlobalSampler Method Definitions
//

Float GlobalSampler::get_1D()
{
	if (dimension >= arrayStartDim && dimension < arrayEndDim)
		dimension = arrayEndDim;
	return sample_dimension(intervalSampleIndex, dimension++);
}

Point2f GlobalSampler::get_2D()
{
	if (dimension + 1 >= arrayStartDim && dimension < arrayEndDim)
		dimension = arrayEndDim;
	Point2f p(sample_dimension(intervalSampleIndex, dimension),
			  sample_dimension(intervalSampleIndex, dimension + 1));
	dimension += 2;
	return p;
}

/*
void GlobalSampler::start_pixel(const Point2i& p)
{
	Sampler::start_pixel(p);
	dimension = 0;
	intervalSampleIndex = get_index_for_sample(0);

	// Compute _arrayEndDim_ for dimensions used for array samples
	// sample[xFilm,yFilm,time,uLens,vLens,[array],[array],[array],...]
	arrayEndDim =
		arrayStartDim + sampleArray_1D.size() + 2 * sampleArray_2D.size();

	// Compute 1D array samples for _GlobalSampler_
	for (size_t i = 0; i < subArraySizes_1D.size(); ++i)
	{
		int nSamples = subArraySizes_1D[i] * samples_PerPixel;
		for (int j = 0; j < nSamples; ++j) 
		{
			int64_t index = get_index_for_sample(j);
			//计算 sampleArray_1D 的样本
			sampleArray_1D[i][j] = sample_dimension(index, arrayStartDim + i);
		}
	}

	// Compute 2D array samples for _GlobalSampler_
	int dim = arrayStartDim + subArraySizes_1D.size();
	for (size_t i = 0; i < subArraySizes_2D.size(); ++i) 
	{
		int nSamples = subArraySizes_2D[i] * samples_PerPixel;
		for (int j = 0; j < nSamples; ++j) 
		{
			int64_t idx = get_index_for_sample(j);
			//计算 sampleArray_2D 的样本
			sampleArray_2D[i][j].x = sample_dimension(idx, dim);
			sampleArray_2D[i][j].y = sample_dimension(idx, dim + 1);
		}
		dim += 2;
	}
	CHECK_EQ(arrayEndDim, dim);
}
*/

bool GlobalSampler::next_sample()
{
	dimension = 0;
	intervalSampleIndex = get_index_for_sample(currentPixel_SampleIndex + 1);
	return Sampler::next_sample();
}

bool GlobalSampler::set_SampleIndex(int64_t sampleNum)
{
	dimension = 0;
	intervalSampleIndex = get_index_for_sample(sampleNum);
	return Sampler::set_SampleIndex(sampleNum);
}

}	// namespace valley