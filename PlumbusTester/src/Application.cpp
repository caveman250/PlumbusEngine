#include <gui/GuiManager.h>
#include "plumbus.h"
#include "Application.h"
#include "TestManager.h"
#include "TesterScene.h"

namespace plumbus::tester
{
	Application::Application()
		: BaseApplication()
	{
		SetAppName("PlumbusTester");
	}

	void Application::Cleanup()
	{
		BaseApplication::Cleanup();
		TestManager::Get().ShutdownActiveTest();
	}

}