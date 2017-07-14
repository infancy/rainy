#include"api.h"

using namespace valley;

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);

	//unique_ptr<Scene> scene(valley_create_scene());
	Scene* p = valley_create_scene();

	Film* film{ new Film(800, 800, 100) };
	//std::unique_ptr<Camera> camera{new PerspectiveCamera;
	Point3f eye(0, 0, -200), tar(-10, 0, 0);
	Vector3f up(0, 1, 0);
	std::unique_ptr<Camera> camera{ new Pinhole(eye, tar, up, 50, film) };

	for (int y = 0; y < 800; y++)
	{
		for (int x = 0; x < 800; x++)
		{
			CameraSample cs;
			cs.pFilm.x = x, cs.pFilm.y = y;
			Ray ray;
			camera->generate_ray(cs, &ray);
			SurfaceIsect isect;

			//cout << "x: " << x << " y: " << y << "\n";
			if (p->intersect(ray, &isect))
			{
				Normal3f n = Normalize(isect.n);
				camera->film->operator()(x, y) = Color((n.x + 1) / 2, (n.y + 1) / 2, (n.z + 1) / 2);
				//camera->film->operator()(x, y) = Color(1, 1, 1);
			}
		}
	}
	delete p;
	return 0;
}