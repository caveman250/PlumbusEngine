#include "plumbus.h"
#include "tests/DeferredLights/DeferredLights.h"
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
#include "renderer/vk/MaterialInstance.h"
#include "renderer/vk/shader_compiler/ShaderSettings.h"


namespace plumbus::tester::tests 
{
	
	DeferredLights::DeferredLights()
		: Test()
		, m_LightTime(0)
		, m_LightsPaused(false)
		, m_LightSpeed(1.0f)
		, m_LightHeight(-4.f)
		, m_LightRadius(20.f)
		, m_LightsDistanceFromCenter(7.f)
		, m_DeferredLightMaterial(new vk::Material("shaders/shader.vert", "shaders/shader.frag"))
	{
		m_DeferredLightMaterial->Setup();
	}

	DeferredLights::~DeferredLights()
	{
	}

	void DeferredLights::Init()
	{
		TesterScene* scene = static_cast<TesterScene*>(Application::Get().GetScene());

		if (Camera* camera = scene->GetCamera())
		{
			camera->SetPosition(glm::vec3(0.f, 11.f, -35.f));
			camera->SetRotation(glm::vec3(-15.f, 0.f, 0.0f));
		}

		GameObject* plane = new GameObject("plane");
		scene->AddGameObject(plane->
			AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "stonefloor_color", "stonefloor_normal"))->
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		plane->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);

        GameObject* plane2 = new GameObject("plane2");
        scene->AddGameObject(plane2->
                AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "stonefloor_color", "stonefloor_normal"))->
                AddComponent<TranslationComponent>(new TranslationComponent())
        );
        plane2->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);
        plane2->GetComponent<TranslationComponent>()->SetRotation(glm::vec3(0.f, 0.f, glm::half_pi<float>()));
        plane2->GetComponent<TranslationComponent>()->SetTranslation(glm::vec3(-10.f, -10.f, 0.f));

        GameObject* plane3 = new GameObject("plane3");
        scene->AddGameObject(plane3->
                AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "stonefloor_color", "stonefloor_normal"))->
                AddComponent<TranslationComponent>(new TranslationComponent())
        );
        plane3->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);
        plane3->GetComponent<TranslationComponent>()->SetRotation(glm::vec3(0.f, 0.f, -glm::half_pi<float>()));
        plane3->GetComponent<TranslationComponent>()->SetTranslation(glm::vec3(10.f, -10.f, 0.f));

        GameObject* plane4 = new GameObject("plane4");
        scene->AddGameObject(plane4->
                AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "stonefloor_color", "stonefloor_normal"))->
                AddComponent<TranslationComponent>(new TranslationComponent())
        );
        plane4->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);
        plane4->GetComponent<TranslationComponent>()->SetRotation(glm::vec3(-glm::half_pi<float>(), 0.f, 0.f));
        plane4->GetComponent<TranslationComponent>()->SetTranslation(glm::vec3(0.f, -10.f, -10.f));

		GameObject* knight = new GameObject("Knight");
		scene->AddGameObject(knight->
			AddComponent<ModelComponent>(new ModelComponent("models/armor.dae", "color", "normal"))->
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		knight->GetComponent<TranslationComponent>()->SetTranslation(glm::vec3(0, -2.4f, 0));
		knight->GetComponent<TranslationComponent>()->SetRotation(glm::vec3(0, -glm::half_pi<float>(), 0));
		knight->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);

		//Lights
		glm::vec3 colours[] =
		{
			glm::vec3(1.5f),
			glm::vec3(3.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 3.f),
			glm::vec3(3.0f, 3.0f, 0.0f),
			glm::vec3(0.0f, 3.0f, 1.f),
			glm::vec3(3.0f, 2.f, 1.f)
		};

		for (int i = 0; i < 6; ++i)
		{
			GameObject* light = new GameObject("Light " + std::to_string(i));
			scene->AddGameObject(light->
				AddComponent<TranslationComponent>(new TranslationComponent())->
				AddComponent<LightComponent>(new LightComponent()));

			light->GetComponent<LightComponent>()->AddPointLight(colours[i], m_LightRadius);
		}

		BaseApplication::Get().GetScene()->LoadAssets();

#if ENABLE_IMGUI
		vk::VulkanRenderer* vkRenderer = vk::VulkanRenderer::Get();
		m_AlbedoTextureDescSet = vkRenderer->GetImGui()->CreateImGuiTextureMaterialInstance(vkRenderer->GetDeferredFramebuffer()->GetSampler(), vkRenderer->GetDeferredFramebuffer()->GetAttachment("colour").m_ImageView, false);
		m_NormalsTextureDescSet = vkRenderer->GetImGui()->CreateImGuiTextureMaterialInstance(vkRenderer->GetDeferredFramebuffer()->GetSampler(), vkRenderer->GetDeferredFramebuffer()->GetAttachment("normal").m_ImageView, false);
		m_WorldPosTextureDescSet = vkRenderer->GetImGui()->CreateImGuiTextureMaterialInstance(vkRenderer->GetDeferredFramebuffer()->GetSampler(), vkRenderer->GetDeferredFramebuffer()->GetAttachment("position").m_ImageView, false);
#endif
	}

	void DeferredLights::Update()
	{
		if (!m_LightsPaused)
		{
			int index = 0;
			float degrees = 0.f;
			m_LightTime += Application::Get().GetDeltaTime() * m_LightSpeed;
			for (GameObject* obj : Application::Get().GetScene()->GetObjects())
			{
				if (LightComponent* lightComp = obj->GetComponent<LightComponent>())
				{
					TranslationComponent* comp = obj->GetComponent<TranslationComponent>();
					float radians = (glm::radians(degrees)) + (float)m_LightTime;
					float x = m_LightsDistanceFromCenter * cos(radians);
					float z = m_LightsDistanceFromCenter * sin(radians);

					comp->SetTranslation(glm::vec3(x, m_LightHeight, z));
					index++;
					degrees += 60.f;

					for (Light* light : lightComp->GetLights())
					{
						if (light->GetType() == LightType::Point)
						{
							if (PointLight* pointLight = static_cast<PointLight*>(light))
							{
								pointLight->SetRadius(m_LightRadius);
							}
						}
					}
				}
			}
		}
	}

	void DeferredLights::Shutdown()
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

	void DeferredLights::OnGui()
	{
		ImGui::Text("Deferred Lights");
		ImGui::Checkbox("Pause Lights", &m_LightsPaused);
		ImGui::DragFloat("Light Speed", &m_LightSpeed, 0.01f, 0.f, 20.f);
		ImGui::DragFloat("Light Height", &m_LightHeight, 0.01f, -10.f, 50.f);
		ImGui::DragFloat("Light Radius", &m_LightRadius, 0.01f, -10.f, 50.f);
		ImGui::DragFloat("Light Distance From Center", &m_LightsDistanceFromCenter, 0.01f, 0.f, 20.f);
#if !PL_DIST
		ImGui::Image(m_AlbedoTextureDescSet.get(), ImVec2(400, 225), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0), false);
		ImGui::Image(m_NormalsTextureDescSet.get(), ImVec2(400, 225), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0), false);
		ImGui::Image(m_WorldPosTextureDescSet.get(), ImVec2(400, 225), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0), false);
#endif
	}

}
