#include"api.h"

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);

	valley::valley_create_integrator();

	return 0;
}