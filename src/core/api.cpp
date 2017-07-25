#include"api.h"

namespace valley
{

//什么时候用指针 && 指针or智能指针
//破坏封装

void valley_interactive(shared_ptr<Integrator>& ior, const Scene& scene)
{
	RandomSampler sampler(1);
	ior->preprocess(scene, sampler);
	int x = 0, y = 0;

	while (std::cin >> x >> y)
		if (y >= 0 && y < ior->camera->film->height && x >= 0 && x < ior->camera->film->width)
		{
			std::cout << "\n";
			Ray ray;
			ior->camera->generate_ray(sampler.get_CameraSample(x, y), &ray);
			Spectrum L = ior->Li(ray, scene, sampler);
			std::cout << "\n";
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
#endif
};
//valley_CornellBox_integrator
Integrator* valley_create_integrator()
{
	Film* film{ new Film(512, 512, new BoxFilter) };
	Point3f eye(0, 0, -130), tar(0, 0, 0);
	Vector3f up(0, 1, 0);
	auto camera{ make_shared<PerspectiveCamera>(eye, tar, up, 60, film) };

	srand(time(nullptr));
	auto sampler{ make_shared<RandomSampler>(4, rand()) };

	//选择策略
	//return  make_shared<Integrator>(new RayCast(camera, sampler, 1));
	return new PathTracing(camera, sampler);
}
//valley_CornellBox_scene
shared_ptr<Scene> valley_create_scene()
{
	//material

	auto sigma(make_shared<ConstantTexture<Float>>(Float(1.f)));

	//diffuse material
	auto green_kd(make_shared<ConstantTexture<Spectrum>>(Spectrum{ 0.156863f, 0.803922f, 0.172549f }));
	auto green_mat(make_shared<Matte>(green_kd, sigma));

	auto red_kd(make_shared<ConstantTexture<Spectrum>>(Spectrum{ 0.803922f, 0.152941f, 0.152941f }));
	auto red_mat(make_shared<Matte>(red_kd, sigma));

	auto blue_kd(make_shared<ConstantTexture<Spectrum>>(Spectrum{ 0.156863f, 0.172549f, 0.803922f }));
	auto blue_mat(make_shared<Matte>(blue_kd, sigma));

	auto white_kd(make_shared<ConstantTexture<Spectrum>>(Spectrum{ 0.803922f }));
	auto white_mat(make_shared<Matte>(white_kd, sigma));

	//mirror material
	auto mirror_ball_kd(make_shared<ConstantTexture<Spectrum>>(Spectrum{ 1.f }));
	auto mirror_ball(make_shared<MirrorMaterial>(mirror_ball_kd));

	auto mirror_wall_kd(make_shared<ConstantTexture<Spectrum>>(Spectrum{ 0.803922f }));
	auto mirror_wall(make_shared<MirrorMaterial>(mirror_wall_kd));

	//glossy material


	//shape

	//ball
	//auto ball = make_shared<Sphere>(o2w, w2o, reverseOrientation, radius, zmin, zmax, phimax);
	Transform* m_ball(new Transform(Translate(Vector3f(0, -20, 0))));
	shared_ptr<Sphere> ball{ new Sphere(m_ball, false, 30.f) };

	//wall-back
	Transform* m_back(new Transform(Translate(Vector3f(0, 0, 50))));
	shared_ptr<Rectangle> wall_back{ new Rectangle(m_back, true, 100, 100) };
	
	//wall-right
	Transform* m_right(new Transform(Translate(Vector3f(50, 0, 0))*Rotate(-90, Vector3f(0, 1, 0))));
	shared_ptr<Rectangle> wall_right{ new Rectangle(m_right, false, 100, 100) };

	//wall-left
	Transform* m_left(new Transform(Translate(Vector3f(-50, 0, 0))*Rotate(90, Vector3f(0, 1, 0))));
	shared_ptr<Rectangle> wall_left{ new Rectangle(m_left, false, 100, 100) };

	//wall-down
	Transform* m_down(new Transform(Translate(Vector3f(0, -50, 0))*Rotate(-90, Vector3f(1, 0, 0))));
	shared_ptr<Rectangle> wall_down{ new Rectangle(m_down, false, 100, 100) };

	//wall-up
	Transform* m_up(new Transform(Translate(Vector3f(0, 50, 0))*Rotate(90, Vector3f(1, 0, 0))));
	shared_ptr<Rectangle> wall_up{ new Rectangle(m_up, false, 100, 100) };





	//primitive
	vector<shared_ptr<Primitive>> primitive;

	primitive.push_back(make_unique<GeometricPrimitive>(ball, mirror_ball));

	primitive.push_back(make_unique<GeometricPrimitive>(wall_back, white_mat));
	primitive.push_back(make_unique<GeometricPrimitive>(wall_left, green_mat));
	primitive.push_back(make_unique<GeometricPrimitive>(wall_right, red_mat));
	primitive.push_back(make_unique<GeometricPrimitive>(wall_down, mirror_wall));

	primitive.push_back(make_unique<GeometricPrimitive>(wall_up, white_mat));


	//light

	//Arealight
	Transform* ml_up(new Transform(Translate(Vector3f(0, 49.99, 0))*Rotate(90, Vector3f(1, 0, 0))));
	Transform ml_upp(Transform(Translate(Vector3f(0, 49.99, 0))*Rotate(90, Vector3f(1, 0, 0))));
	shared_ptr<Shape> light_up{ new Rectangle(ml_up, false, 30, 30) };
	//区域光必须进行多次采样
	shared_ptr<AreaLight> area_light{ new DiffuseAreaLight(ml_upp, Spectrum(50), 4, light_up) };

	//带区域光源的 shape ： L = Le（area） + Li(material->brdf),两者是不关联的
	primitive.push_back(make_unique<GeometricPrimitive>(light_up, white_mat, area_light));

	//LightToWorld
	Transform ml_point(Transform(Translate(Vector3f(0, 45, 0))));
	shared_ptr<Light> point_light{ new PointLight(ml_point, Spectrum(5000,5000,5000)) };

	//Transform ml_distance;
	//最后的Vector3f表示的是光源所处的方向，而不是光源发出的光的方向
	//shared_ptr<Light> distance_light{ new DistantLight(ml_distance, Spectrum(0,2,2), Vector3f(-1, -1, 0)) };

	std::vector<std::shared_ptr<Light>> lights;
	lights.push_back(area_light);
	//lights.push_back(point_light);
	//lights.push_back(distance_light);

	//scene
	return  make_shared<Scene>(new Accelerator(primitive), lights);
}

}	//namespace valley