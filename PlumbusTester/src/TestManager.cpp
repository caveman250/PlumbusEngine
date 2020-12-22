#include "plumbus.h"

#include "TestManager.h"
#include "tests/Test.h"
#include "tests/DeferredLights/DeferredLights.h"
#include "tests/SponzaScene/SponzaScene.h"
#include "tests/Shadows/Shadows.h"

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
		if (m_ActiveTest)
		{
			m_ActiveTest->Shutdown();
			delete m_ActiveTest;
			m_ActiveTest = nullptr;
		}
	}

	void TestManager::Update()
	{
		if (m_ActiveTest)
		{
			m_ActiveTest->Update();
		}
		else
		{
			BeginTest<tests::DeferredLights>();
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

				if (ImGui::Button("Sponza Scene"))
				{
					BeginTest<tests::SponzaScene>();
				}

				if (ImGui::Button("Shadows"))
				{
					BeginTest<tests::Shadows>();
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
