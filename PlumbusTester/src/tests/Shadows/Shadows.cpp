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
#include "renderer/vk/ShadowManager.h"

namespace plumbus::tester::tests 
{
	
	Shadows::Shadows()
		: Test()
		, m_DeferredLightMaterial(new vk::Material("shaders/shader.vert", "shaders/shader.frag"))
	{
		m_DeferredLightMaterial->Setup();
	}

	Shadows::~Shadows()
	{
	}

	void Shadows::Init()
	{
		TesterScene* scene = static_cast<TesterScene*>(Application::Get().GetScene());

		if (Camera* camera = scene->GetCamera())
		{
			camera->SetPosition(glm::vec3(20.f, 6.f, 0.f));
			camera->SetRotation(glm::vec3(-30.f, 90.f, 0.0f));
		}

		GameObject* plane = new GameObject("plane");
		scene->AddGameObject(plane->
#if PL_PLATFORM_ANDROID
			AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "stonefloor_color", "stonefloor_normal"))->
#else
             AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "stonefloor_color", "stonefloor_normal"))->
#endif
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		plane->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);

		GameObject* knight = new GameObject("Knight");
		scene->AddGameObject(knight->
#if PL_PLATFORM_ANDROID
         AddComponent<ModelComponent>(new ModelComponent("models/armor.dae", "color", "normal"))->
#else
         AddComponent<ModelComponent>(new ModelComponent("models/armor.dae", "color", "normal"))->
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
#if !PL_DIST
		if(!m_ShadowTextureImGui)
		{
			std::vector<vk::Shadow*>& shadows = vk::ShadowManager::Get()->GetShadows();
			if (shadows.size() > 0)
			{
				vk::VulkanRenderer* vkRenderer = vk::VulkanRenderer::Get();
				m_ShadowTextureImGui = vkRenderer->GetImGui()->CreateImGuiTextureMaterialInstance(shadows.back()->GetFrameBuffer()->GetSampler(), shadows.back()->GetFrameBuffer()->GetAttachment("depth").m_ImageView, true);
			}
		}
#endif
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

#if !PL_DIST
		if(m_ShadowTextureImGui)
			ImGui::Image(m_ShadowTextureImGui.get(), ImVec2(400, 225), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0), false);
#endif
		
	}

}
