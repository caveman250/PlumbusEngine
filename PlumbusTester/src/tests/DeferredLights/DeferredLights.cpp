#include "plumbus.h"
#include "tests/DeferredLights/DeferredLights.h"
#include "BaseApplication.h"
#include "renderer/base/Window.h"
#include "renderer/base/Renderer.h"
#include "components/ModelComponent.h"
#include "components/TranslationComponent.h"
#include "components/PointLightComponent.h"
#include "GameObject.h"

#include "Application.h"
#include "TesterScene.h"
#include "renderer/base/Material.h"
#if VULKAN_RENDERER// see TODO in constructor
#include "renderer/vk/Material.h"
#endif

namespace plumbus::tester::tests 
{
	
	DeferredLights::DeferredLights()
		: Test()
		, m_LightTime(0)
		, m_LightsPaused(false)
		, m_LightSpeed(1.0f)
		, m_LightsDistanceFromCenter(5.f)
#if VULKAN_RENDERER //TODO there should probably me some kind of manager that just returns the correct type of material
		, m_DeferredLightMaterial(new vk::Material("shaders/bin/shader.vert.spv", "shaders/bin/shader.frag.spv"))
#endif
	{

	}

	DeferredLights::~DeferredLights()
	{
		int lol = 1;
	}

	void DeferredLights::Init()
	{
		TesterScene* scene = static_cast<TesterScene*>(Application::Get().GetScene());

		GameObject* obj = new GameObject("Knight");
		scene->AddGameObject(obj->
			AddComponent<ModelComponent>(new ModelComponent("models/armor.dae", "textures/color_bc3_unorm.ktx", "textures/normal_bc3_unorm.ktx"))->
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		obj->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);

		GameObject* plane = new GameObject("Plane");
		scene->AddGameObject(plane->
			AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "textures/stonefloor01_color_bc3_unorm.ktx", "textures/stonefloor01_normal_bc3_unorm.ktx"))->
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		plane->GetComponent<TranslationComponent>()->SetTranslation(glm::vec3(0, 2.3, 0));
		plane->GetComponent<ModelComponent>()->SetMaterial(m_DeferredLightMaterial);

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
				AddComponent<PointLightComponent>(new PointLightComponent(colours[i], 10.0f)));
		}

		BaseApplication::Get().GetScene()->LoadAssets();
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
				if (PointLightComponent* lightComp = obj->GetComponent<PointLightComponent>())
				{
					TranslationComponent* comp = obj->GetComponent<TranslationComponent>();
					float radians = (glm::radians(degrees)) + (float)m_LightTime;
					float x = m_LightsDistanceFromCenter * cos(radians);
					float z = m_LightsDistanceFromCenter * sin(radians);

					comp->SetTranslation(glm::vec3(x, 0.f, z));
					index++;
					degrees += 60.f;
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
	}

	void DeferredLights::OnGui()
	{
		ImGui::Text("Deferred Lights");
		ImGui::Checkbox("Pause Lights", &m_LightsPaused);
		ImGui::DragFloat("Light Speed", &m_LightSpeed, 0.01f, 0.f, 20.f);
		ImGui::DragFloat("Light Distance From Center", &m_LightsDistanceFromCenter, 0.01f, 0.f, 20.f);
	}

}