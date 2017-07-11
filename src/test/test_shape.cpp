#include"api.h"

using namespace valley;

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);
	
	Transform m(Matrix4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 100,
		0, 0, 0, 1));
	Transform mInv = Inverse(m);
	shared_ptr<Sphere> sphere{ new Sphere(&m, &mInv, false, 50.f) };
	
	/*
	Transform m1(Matrix4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 200,
		0, 0, 0, 1));
	Transform mInv1 = Inverse(m1);
	shared_ptr<Rectangle> rectangle{ new Rectangle(&m1, &mInv1, false, Point3f(0, 0, 0),
		Vector3f(0, 0, 100), Vector3f(0, 100, 0)) };
		*/
	Film* film{ new Film(800, 600, 100) };
	//std::unique_ptr<Camera> camera{new PerspectiveCamera;
	Point3f eye(0, 0, 0), tar(0, 0, 1);
	Vector3f up(0, 1, 0);
	std::unique_ptr<Camera> camera{ new Pinhole(eye, tar, up, 50, film) };

	long i = 0;
	for (int y = 401; y < 600; y++)
	{
		for (int x = 1; x < 800; x++)
		{
			CameraSample cs;
			cs.pFilm.x = x, cs.pFilm.y = y;
			Ray ray;
			camera->generate_ray(cs, &ray);
			SurfaceIsect isect;
			Float max = Infinity;
			++i;
			//cout << i << "\n";
			if (sphere->intersect(ray, &isect))
			{
				Normal3f n = Normalize(isect.n);
				camera->film->operator()(x, y) = Color((n.x + 1) / 2, (n.y + 1) / 2, (n.z + 1) / 2);
			}
		}
	}

	return 0;
}