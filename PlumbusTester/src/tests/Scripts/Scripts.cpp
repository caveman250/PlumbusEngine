#include "plumbus.h"
#include "Scripts.h"

#include "Application.h"
#include "GameObject.h"
#include "TesterScene.h"
#include "components/ScriptComponent.h"
#include "components/TranslationComponent.h"
#include "renderer/vk/Material.h"

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

		BaseApplication::Get().GetScene()->LoadAssets();
	}

	void Scripts::Update()
	{
		
	}

	void Scripts::Shutdown()
	{
		BaseApplication::Get().GetScene()->ClearObjects();
	}

	void Scripts::OnGui()
	{
		
	}
}
