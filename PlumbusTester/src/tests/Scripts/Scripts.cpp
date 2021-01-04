#include "plumbus.h"
#include "Scripts.h"

#include "Application.h"
#include "GameObject.h"
#include "TesterScene.h"
#include "components/ScriptComponent.h"
#include "components/TranslationComponent.h"
#include "components/LightComponent.h"
#include "renderer/vk/Material.h"
#include "renderer/vk/VulkanRenderer.h"

namespace plumbus::tester::tests
{
	Scripts::Scripts()
		: Test()
		, m_DeferredLightMaterial(new vk::Material("shaders/shader.vert", "shaders/shader.frag"))
	{
		m_DeferredLightMaterial->Setup();
	}

	void Scripts::Init()
	{
		TesterScene* scene = static_cast<TesterScene*>(Application::Get().GetScene());
		
		if (Camera* camera = scene->GetCamera())
		{
			camera->SetPosition(glm::vec3(0.f, 11.f, -35.f));
			camera->SetRotation(glm::vec3(-15.f, 0.f, 0.0f));
		}

		GameObject* knight = new GameObject("Knight");
		scene->AddGameObject(knight->
            AddComponent<components::ModelComponent>(new components::ModelComponent("models/armor.dae", "color", "normal"))->
            AddComponent<components::TranslationComponent>(new components::TranslationComponent())->
            AddComponent<components::ScriptComponent>(new components::ScriptComponent("TestClass.cs", "MonoTester"))
        );

		knight->GetComponent<components::TranslationComponent>()->SetTranslation(glm::vec3(0, -2.4f, 0));
		knight->GetComponent<components::TranslationComponent>()->SetRotation(glm::vec3(0.f, 0.f, 0.f));
		knight->GetComponent<components::ModelComponent>()->SetMaterial(m_DeferredLightMaterial);

        GameObject* light = new GameObject("light");
        scene->AddGameObject(light->
                AddComponent<components::LightComponent>(new components::LightComponent()));
        light->GetComponent<components::LightComponent>()->AddDirectionalLight(glm::vec3(1.f, 1.f, 0.8f), glm::vec3(1.f, -1.f, 1.f), false);

        BaseApplication::Get().GetScene()->LoadAssets();
	}

	void Scripts::Update()
	{
		
	}

	void Scripts::Shutdown()
	{
        vkDeviceWaitIdle(vk::VulkanRenderer::Get()->GetDevice()->GetVulkanDevice());
        for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
        {
            if (components::ModelComponent* component = obj->GetComponent<components::ModelComponent>())
            {
                component->Cleanup();
            }
        }

        BaseApplication::Get().GetScene()->ClearObjects();

        m_DeferredLightMaterial.reset();
	}

	void Scripts::OnGui()
	{
		
	}
}
