#include "plumbus.h"
#include "Application.h"
#include "TestManager.h"

namespace plumbus::tester
{
	Application::Application()
		: BaseApplication()
	{
	}

	void Application::OnGui()
	{
		TestManager::Get().OnGui();
	}

}
