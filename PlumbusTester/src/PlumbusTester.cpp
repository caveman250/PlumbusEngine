#include "plumbus.h"
#include "Application.h"
#include "TesterScene.h"

int app_argc;
char** app_argv;

int main(int argc, char** argv)
{
	app_argc = argc;
	app_argv = argv;
	using namespace plumbus::tester;

	Application::CreateInstance();
	Application::Get().CreateScene<TesterScene>();
	Application::Get().Run();

	return 0;
}
