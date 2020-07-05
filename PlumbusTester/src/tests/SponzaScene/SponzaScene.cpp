#include "plumbus.h"
#include "tests/SponzaScene/SponzaScene.h"
#include "BaseApplication.h"
#include "renderer/base/Window.h"
#include "renderer/base/Renderer.h"
#include "components/ModelComponent.h"
#include "components/TranslationComponent.h"
#include "components/LightComponent.h"
#include "GameObject.h"

#include "Application.h"
#include "TesterScene.h"
#include "renderer/base/Material.h"
#if VULKAN_RENDERER// see TODO in constructor
#include "renderer/vk/Material.h"
#include "renderer/vk/VulkanRenderer.h"
#endif



namespace plumbus::tester::tests 
{
	
	SponzaScene::SponzaScene()
		: Test()
#if VULKAN_RENDERER //TODO there should probably me some kind of manager that just returns the correct type of material
		, m_DeferredLightMaterial(new vk::Material("shaders/shader.vert.spv", "shaders/shader.frag.spv"))
#endif
	{

	}

	SponzaScene::~SponzaScene()
	{
	}

	void SponzaScene::Init()
	{
		TesterScene* scene = static_cast<TesterScene*>(Application::Get().GetScene());

		GameObject* sponza = new GameObject("Sponza");
		scene->AddGameObject(sponza->
			AddComponent<ModelComponent>(new ModelComponent("models/sponza.dae", "color_bc3_unorm.ktx", "normal_bc3_unorm.ktx"))->
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		sponza->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);

        //light
        GameObject* light = new GameObject("Directional Light");
        scene->AddGameObject(light->
				AddComponent<TranslationComponent>(new TranslationComponent())->
				AddComponent<LightComponent>(new LightComponent()));

        light->GetComponent<LightComponent>()->AddDirectionalLight(glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.f, -0.8f, 1.f));

		BaseApplication::Get().GetScene()->LoadAssets();
	}

	void SponzaScene::Update()
	{
		
	}

	void SponzaScene::Shutdown()
	{
		for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
		{
			if (ModelComponent* component = obj->GetComponent<ModelComponent>())
			{
				component->Cleanup();
			}
		}

		BaseApplication::Get().GetScene()->ClearObjects();
		BaseApplication::Get().GetRenderer()->OnModelRemovedFromScene();

		m_DeferredLightMaterial->Destroy();
	}

	void SponzaScene::OnGui()
	{
		ImGui::Text("Sponza Scene");
	}

}