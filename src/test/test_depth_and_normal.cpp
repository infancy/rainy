#include"valley.h"
#include"film.h"
#include"camera.h"
#include"shape/sphere.h"
using namespace valley;

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);
	std::unique_ptr<Sphere> sphere{new Sphere{Point3f{0.f, 0.f, 100.f}, 50.f}};
	Film* film{ new Film(800, 600, 100) };
	//std::unique_ptr<Camera> camera{new PerspectiveCamera;
	Point3f eye(0, 0, 0), tar(0, 0, 1);
	Vector3f up(0, 1, 0);
	Camera* camera {new Pinhole(eye, tar, up, 50, film)};
	//Camera* camera{ new OrthographicCamera(t, b, 0.f, 50.f, film) };
	/*
	std::unique_ptr<Integrator> integrator;

	integrator->render(sphere);
	*/
	float maxDepth = 50;
	for (int y = 0; y < 600; y++)
	{
		for (int x = 0; x < 800; x++)
		{
			CameraSample cs;
			cs.pFilm.x = x, cs.pFilm.y = y;
			Ray ray;
			camera->generate_ray(cs, &ray);
			Isect isect;

			if (sphere->intersect(ray, isect))
			{
				//float depth = 2.f - isect.p.z / maxDepth;
				Normal3f n = Normalize(isect.n);
				camera->film->operator()(x, y) = Color4f((n.x + 1)/2, (n.y + 1) / 2, (n.z + 1) / 2);
			}
		}
	}

	delete film;
	film = nullptr;
	delete camera;

	return 0;
}