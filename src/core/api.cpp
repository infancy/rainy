#include"api.h"

namespace valley
{

Scene* valley_create_scene()
{
	vector<shared_ptr<Primitive>> primitive;

	shared_ptr<ConstantTexture<Color>> kd(new ConstantTexture<Color>(Color{ 1.f, 0.f, 1.f }));
	shared_ptr<ConstantTexture<Float>> sigma(new ConstantTexture<Float>(Float(1.f)));
	shared_ptr<Matte> matte(new Matte(kd, sigma));

	shared_ptr<Transform> m(new Transform(Matrix4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1)));
	shared_ptr<Transform> mInv(new Transform(m->GetMatrix(), m->GetInverseMatrix()));
	shared_ptr<Sphere> sphere{ new Sphere(m, mInv, false, 50.f) };
	primitive.push_back(make_unique<GeometricPrimitive>(sphere, matte));

	shared_ptr<Transform> m1(new Transform(Matrix4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1)));
	shared_ptr<Transform> mInv1(new Transform(m1->GetMatrix(), m1->GetInverseMatrix()));
	shared_ptr<Rectangle> rectangle{ new Rectangle(m1, mInv1, false, Point3f(0, 0, 0),
		Vector3f(0, 0, 100), Vector3f(0, 100, 0)) };

	primitive.push_back(make_unique<GeometricPrimitive>(rectangle, matte));

	std::vector<std::shared_ptr<Light>> lights;
	return new Scene(new Accelerator(primitive), lights);
}

}	//namespace valley