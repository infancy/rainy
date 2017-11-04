#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef RAINY_CAMERA_PERSPECTIVE_H
#define RAINY_CAMERA_PERSPECTIVE_H

#include"rainy.h"
#include"camera.h"

namespace rainy
{

/*								   |
								  *|
Image(像平面)		       *   *   |
|		  Lens      * |	   *	   |
|           |*		  | *		   |
|		  * |      *  |			   |
|		*   |  *      |			   |
|	  *    *|         |          Focal
|	*   *   |         |
| *  *      |         |
|*	    	|		  |
| 		    		  |
					View（视平面）
					
通过穿过透镜中心的中心光线确定位置，而后生成主光线进行着色
虽然主光线可能未通过视平面的相应像素的位置，但这并没有关系
当薄透镜的半径为 0 时，则透镜相机变成了针孔相机
*/

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera(const Point3f eye, const Point3f target, const Vector3f up,
		Float fovy, Film* film, Float lensRadius = 0.f, Float focalDistance = 1e6);

	Float generate_ray(const CameraSample& sample, Ray* ray) const;

	Spectrum We(const Ray& ray, Point2f* pRaster2 = nullptr) const;
	void pdf_We(const Ray& ray, Float* pdfPos, Float* pdfDir) const;
	Spectrum sample_Wi(const Interaction& ref, const Point2f& u,
		Vector3f* wi, Float* pdf, Point2f* pRaster, Visibility* vis) const;

private:
	Transform raster_to_camera, camera_to_world, world_to_raster;

	Float lensRadius, focalDistance;	//透镜半径、焦距
	Float Area;	//胶片实际大小
};

}	//namespace rainy


#endif //RAINY_CAMERA_PERSPECTIVE_H
