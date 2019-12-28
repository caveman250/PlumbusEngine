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
		Scene::OnUpdate();
		TestManager::Get().Update();
	}

}