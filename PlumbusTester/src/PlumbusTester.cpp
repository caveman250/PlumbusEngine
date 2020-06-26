#include "plumbus.h"
#include "Application.h"
#include "TesterScene.h"

int main()
{
	using namespace plumbus::tester;

	Application::CreateInstance();
	Application::Get().CreateScene<TesterScene>();
	Application::Get().Run();

	return 0;
}
