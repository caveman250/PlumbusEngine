#include "plumbus.h"

#include "TestManager.h"
#include "tests/Test.h"
#include "tests/DeferredLights/DeferredLights.h"

namespace plumbus::tester
{
	TestManager TestManager::s_Instance;

	TestManager::TestManager()
		: m_ActiveTest(nullptr)
	{

	}

	void TestManager::BeginTestInternal()
	{
		m_ActiveTest->Init();
	}

	void TestManager::ShutdownActiveTest()
	{
		m_ActiveTest->Shutdown();
		delete m_ActiveTest;
		m_ActiveTest = nullptr;
	}

	void TestManager::Update()
	{
		if (m_ActiveTest)
		{
			m_ActiveTest->Update();
		}
	}

	void TestManager::OnGui()
	{
		if (ImGui::Begin("Tests"))
		{
			if (!m_ActiveTest)
			{
				if (ImGui::Button("Deferred Lights"))
				{
					BeginTest<tests::DeferredLights>();
				}
			}
			else
			{
				if (ImGui::Button("Exit Test"))
				{
					ShutdownActiveTest();
				}
				else
				{
					ImGui::Separator();
					m_ActiveTest->OnGui();
				}
			}

			ImGui::End();
		}
	}

}
