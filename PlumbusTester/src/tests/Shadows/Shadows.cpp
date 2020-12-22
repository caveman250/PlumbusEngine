#include "plumbus.h"
#include "tests/Shadows/Shadows.h"
#include "BaseApplication.h"
#include "renderer/vk/Window.h"
#include "components/ModelComponent.h"
#include "components/TranslationComponent.h"
#include "components/LightComponent.h"
#include "GameObject.h"

#include "Application.h"
#include "TesterScene.h"
#include "renderer/vk/Material.h"
#include "renderer/vk/VulkanRenderer.h"
#include "imgui_impl/ImGuiImpl.h"
#include "renderer/vk/DescriptorSet.h"

namespace plumbus::tester::tests 
{
	
	Shadows::Shadows()
		: Test()
		, m_DeferredLightMaterial(new vk::Material("shaders/shader.vert.spv", "shaders/shader.frag.spv"))
	{
		m_DeferredLightMaterial->Setup(vk::Mesh::s_VertexLayout);
	}

	Shadows::~Shadows()
	{
	}

	void Shadows::Init()
	{
		TesterScene* scene = static_cast<TesterScene*>(Application::Get().GetScene());

		if (Camera* camera = scene->GetCamera())
		{
			camera->SetPosition(glm::vec3(10.f, 3.f, -20.f));
			camera->SetRotation(glm::vec3(0.f, 90.f, 90.f));
		}

		GameObject* plane = new GameObject("plane");
		scene->AddGameObject(plane->
#if PL_PLATFORM_ANDROID
			AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "stonefloor_color.astc", "stonefloor_normal.astc"))->
#else
             AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "stonefloor01_color_bc3_unorm.ktx", "stonefloor01_normal_bc3_unorm.ktx"))->
#endif
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		plane->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);

		GameObject* knight = new GameObject("Knight");
		scene->AddGameObject(knight->
#if PL_PLATFORM_ANDROID
         AddComponent<ModelComponent>(new ModelComponent("models/armor.dae", "color.astc", "normal.astc"))->
#else
         AddComponent<ModelComponent>(new ModelComponent("models/armor.dae", "color_bc3_unorm.ktx", "normal_bc3_unorm.ktx"))->
#endif
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		knight->GetComponent<TranslationComponent>()->SetTranslation(glm::vec3(0, -2.4f, 0));
		knight->GetComponent<TranslationComponent>()->SetRotation(glm::vec3(0, -glm::half_pi<float>(), 0));
		knight->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);

        GameObject* light = new GameObject("Light");
        scene->AddGameObject(light->
            AddComponent<LightComponent>(new LightComponent())
		);

        light->GetComponent<LightComponent>()->AddDirectionalLight(glm::vec3(0.8f, 0.8f, 0.6f), glm::vec3(-1.f, -1.f, 1.f), true);

		BaseApplication::Get().GetScene()->LoadAssets();
	}

	void Shadows::Update()
	{
	}

	void Shadows::Shutdown()
	{
		vkDeviceWaitIdle(vk::VulkanRenderer::Get()->GetDevice()->GetVulkanDevice());
		for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
		{
			if (ModelComponent* component = obj->GetComponent<ModelComponent>())
			{
				component->Cleanup();
			}
		}

		BaseApplication::Get().GetScene()->ClearObjects();

		m_DeferredLightMaterial.reset();
	}

	void Shadows::OnGui()
	{
		ImGui::Text("Shadows");
		//TODO
	}

}