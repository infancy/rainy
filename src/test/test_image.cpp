#include"image.h"
#include"color.h"

valley::Color4f p[100][100];

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);

	for (int i = 0; i < 10; ++i)
		for(int j = 0; j < 100; ++j)
		{
			p[10 * i][j] = 1.f;
			p[j][10 * i] = 1.f;
		}
	valley::save_ppm("test.ppm", &p[0][0], 100, 100);

	return 0;
}