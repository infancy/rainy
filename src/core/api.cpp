#include"api.h"

namespace valley
{

//什么时候用指针 && 指针or智能指针
//破坏封装

void valley_interactive(shared_ptr<Integrator>& ior, const Scene& scene)
{
	Sampler sampler(rand());
	ior->preprocess(scene, sampler);
	int x = 0, y = 0;
	Sampler Pixelsampler(rand());
	while (std::cin >> y >> x)
		if (y >= 0 && y < ior->camera->film->height && x >= 0 && x < ior->camera->film->width)
		{
			Ray ray;
			ior->camera->generate_ray(sampler.get_CameraSample(x, y), &ray);
			Color L = ior->Li(ray, scene, Pixelsampler);
		}
		else
			std::cout << "error,should't call position out of film\n";
}

void valley_render()
{
	shared_ptr<Scene> scene = valley_create_scene();
	//shared_ptr<Integrator> integrator = valley_create_integrator(); 不接受等号的重载
	shared_ptr<Integrator> integrator(valley_create_integrator());
#if defined _DEBUG
	valley_interactive(integrator, *scene);
#else
	integrator->render(*scene);
	integrator->camera->film->flush();
#endif
};

Integrator* valley_create_integrator()
{
	Film* film{ new Film(600, 600, 100) };
	//std::unique_ptr<Camera> camera{new PerspectiveCamera;
	Point3f eye(0, 0, -130), tar(0, 0, 0);
	Vector3f up(0, 1, 0);
	shared_ptr<Camera> camera{ new Pinhole(eye, tar, up, 50, film) };
	srand(time(nullptr));
	shared_ptr<Sampler> sampler{ new Sampler(1, rand()) };

	//选择策略
	//return  make_shared<Integrator>(new RayCast(camera, sampler, 1));
	return new RayCast(camera, sampler, 1);
}

shared_ptr<Scene> valley_create_scene()
{

	//material
	shared_ptr<ConstantTexture<Color>> kd(new ConstantTexture<Color>(Color{ 0.5f, 0.5f, 0.5f }));
	shared_ptr<ConstantTexture<Float>> sigma(new ConstantTexture<Float>(Float(1.f)));
	shared_ptr<Matte> matte(new Matte(kd, sigma));

	//shape

	//ball
	Transform* m(new Transform(Translate(Vector3f(0, -20, 0))));
	shared_ptr<Sphere> ball{ new Sphere(m, true, 30.f) };

	//wall-back
	Transform* m_back(new Transform(Translate(Vector3f(0, 0, 50))*Rotate(-90, Vector3f(1, 0, 0))));
	shared_ptr<Rectangle> wall_back{ new Rectangle(m_back, false, 100, 100) };
	
	//wall-right
	Transform* m_right(new Transform(Translate(Vector3f(50, 0, 0))*Rotate(90, Vector3f(0, 0, 1))));
	shared_ptr<Rectangle> wall_right{ new Rectangle(m_right, false, 100, 100) };

	//wall-left
	Transform* m_left(new Transform(Translate(Vector3f(-50, 0, 0))*Rotate(-90, Vector3f(0, 0, 1))));
	shared_ptr<Rectangle> wall_left{ new Rectangle(m_left, false, 100, 100) };

	//wall-down
	Transform* m_down(new Transform(Translate(Vector3f(0, -50, 0))));
	shared_ptr<Rectangle> wall_down{ new Rectangle(m_down, false, 100, 100) };

	//wall-up
	Transform* m_up(new Transform(Translate(Vector3f(0, 50, 0))));
	shared_ptr<Rectangle> wall_up{ new Rectangle(m_up, true, 100, 100) };

	//add to primitive
	vector<shared_ptr<Primitive>> primitive;
	primitive.push_back(make_unique<GeometricPrimitive>(ball, matte));
	primitive.push_back(make_unique<GeometricPrimitive>(wall_back, matte));
	primitive.push_back(make_unique<GeometricPrimitive>(wall_left, matte));
	primitive.push_back(make_unique<GeometricPrimitive>(wall_right, matte));
	primitive.push_back(make_unique<GeometricPrimitive>(wall_down, matte));
	primitive.push_back(make_unique<GeometricPrimitive>(wall_up, matte));

	//Arealight
	Transform* ml_up(new Transform(Translate(Vector3f(0, 49.9, 30))));
	Transform ml_upp(Transform(Translate(Vector3f(0, 49.9, 30))));
	shared_ptr<Shape> light_up{ new Rectangle(ml_up, true, 40, 40) };
	//区域光必须进行多次采样
	shared_ptr<AreaLight> diffuse_light{ new DiffuseAreaLight(ml_upp, Color(30), 1, light_up) };

	primitive.push_back(make_unique<GeometricPrimitive>(light_up, matte, diffuse_light));

	//LightToWorld
	//Transform ml_point(Transform(Translate(Vector3f(0, 0, -50))));
	//shared_ptr<Light> point_light{ new PointLight(ml_point, Color(5000,5000,10)) };

	//Transform ml_distance;
	//最后的Vector3f表示的是光源随处的方向，而不是光源发出的光的方向
	//shared_ptr<Light> distance_light{ new DistantLight(ml_distance, Color(0,2,2), Vector3f(-1, -1, 0)) };

	std::vector<std::shared_ptr<Light>> lights;
	lights.push_back(diffuse_light);
	//lights.push_back(point_light);
	//lights.push_back(distance_light);

	//scene
	return  make_shared<Scene>(new Accelerator(primitive), lights);
}

}	//namespace valley