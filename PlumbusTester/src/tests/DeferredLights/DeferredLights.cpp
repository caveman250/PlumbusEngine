#include "plumbus.h"
#include "tests/DeferredLights/DeferredLights.h"
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
#include "imgui_impl/ImGuiImpl.h"
#endif



namespace plumbus::tester::tests 
{
	
	DeferredLights::DeferredLights()
		: Test()
		, m_LightTime(0)
		, m_LightsPaused(false)
		, m_LightSpeed(1.0f)
		, m_LightHeight(3.f)
		, m_LightRadius(20.f)
		, m_LightsDistanceFromCenter(7.f)
#if VULKAN_RENDERER //TODO there should probably me some kind of manager that just returns the correct type of material
		, m_DeferredLightMaterial(new vk::Material("shaders/shader.vert.spv", "shaders/shader.frag.spv"))
#endif
	{

	}

	DeferredLights::~DeferredLights()
	{
	}

	void DeferredLights::Init()
	{
		TesterScene* scene = static_cast<TesterScene*>(Application::Get().GetScene());

		if (Camera* camera = scene->GetCamera())
		{
			camera->SetPosition(glm::vec3(10.f, 3.f, 0.f));
			camera->SetRotation(glm::vec3(0.f, 90.f, 0.f));
		}

		GameObject* plane = new GameObject("plane");
		scene->AddGameObject(plane->
			AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "stonefloor01_color_bc3_unorm.ktx", "stonefloor01_normal_bc3_unorm.ktx"))->
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		plane->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);

		GameObject* knight = new GameObject("Knight");
		scene->AddGameObject(knight->
			AddComponent<ModelComponent>(new ModelComponent("models/armor.dae", "color_bc3_unorm.ktx", "normal_bc3_unorm.ktx"))->
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		knight->GetComponent<TranslationComponent>()->SetTranslation(glm::vec3(0, -2.1, 0));
		knight->GetComponent<TranslationComponent>()->SetRotation(glm::vec3(0, -glm::half_pi<float>(), 0));
		knight->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);

		//Lights
		glm::vec3 colours[] =
		{
			glm::vec3(1.5f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 2.5f),
			glm::vec3(1.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.2f),
			glm::vec3(1.0f, 0.7f, 0.3f)
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

#if VULKAN_RENDERER
		vk::VulkanRenderer* vkRenderer = static_cast<vk::VulkanRenderer*>(Application::Get().GetRenderer());
		m_AlbedoTextureDescSet = vkRenderer->GetImGui()->AddTexture(vkRenderer->GetOffscreenFramebuffer()->m_ColourSampler, vkRenderer->GetOffscreenFramebuffer()->m_Attachments["colour"].m_ImageView);
		m_NormalsTextureDescSet = vkRenderer->GetImGui()->AddTexture(vkRenderer->GetOffscreenFramebuffer()->m_ColourSampler, vkRenderer->GetOffscreenFramebuffer()->m_Attachments["normal"].m_ImageView);
		m_WorldPosTextureDescSet = vkRenderer->GetImGui()->AddTexture(vkRenderer->GetOffscreenFramebuffer()->m_ColourSampler, vkRenderer->GetOffscreenFramebuffer()->m_Attachments["position"].m_ImageView);
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

	void DeferredLights::OnGui()
	{
		ImGui::Text("Deferred Lights");
		ImGui::Checkbox("Pause Lights", &m_LightsPaused);
		ImGui::DragFloat("Light Speed", &m_LightSpeed, 0.01f, 0.f, 20.f);
		ImGui::DragFloat("Light Height", &m_LightHeight, 0.01f, -10.f, 50.f);
		ImGui::DragFloat("Light Radius", &m_LightRadius, 0.01f, -10.f, 50.f);
		ImGui::DragFloat("Light Distance From Center", &m_LightsDistanceFromCenter, 0.01f, 0.f, 20.f);
#if VULKAN_RENDERER
		ImGui::Image(m_AlbedoTextureDescSet, ImVec2(400, 225), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0), false);
		ImGui::Image(m_NormalsTextureDescSet, ImVec2(400, 225), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0), false);
		ImGui::Image(m_WorldPosTextureDescSet, ImVec2(400, 225), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0), false);
#endif
	}

}