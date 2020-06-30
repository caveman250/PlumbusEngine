#include "plumbus.h"
#include "TesterScene.h"

#include "TestManager.h"
#include "tests/DeferredLights/DeferredLights.h"

namespace plumbus::tester
{
	TesterScene::TesterScene()
	{

	}

	void TesterScene::Init()
	{
		Scene::Init();

		//TestManager::Get().BeginTest<tests::DeferredLights>();
	}

	void TesterScene::Shutdown()
	{
		Scene::Shutdown();
	}

	void TesterScene::OnUpdate()
	{
		//static std::chrono::system_clock::time_point lastFrameTime;
		//static std::chrono::system_clock::time_point currentFrameTime;

		//lastFrameTime = currentFrameTime;
		//currentFrameTime = std::chrono::system_clock::now();

		//std::chrono::duration<double> frameTime = currentFrameTime - lastFrameTime;
		//Log::Info("fps %f", 1 / frameTime.count());
		
		Scene::OnUpdate();
		TestManager::Get().Update();
	}

}