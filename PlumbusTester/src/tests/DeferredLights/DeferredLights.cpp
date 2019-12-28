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


namespace plumbus::tester::tests 
{
	
	DeferredLights::DeferredLights()
	{

	}

	void DeferredLights::Init()
	{
		TesterScene* scene = static_cast<TesterScene*>(Application::Get().GetScene());

		GameObject* obj = new GameObject("Knight");
		scene->AddGameObject(obj->
			AddComponent<ModelComponent>(new ModelComponent("models/armor.dae", "textures/color_bc3_unorm.ktx", "textures/normal_bc3_unorm.ktx"))->
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		GameObject* plane = new GameObject("Plane");
		scene->AddGameObject(plane->
			AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "textures/stonefloor01_color_bc3_unorm.ktx", "textures/stonefloor01_normal_bc3_unorm.ktx"))->
			AddComponent<TranslationComponent>(new TranslationComponent())
		);

		plane->GetComponent<TranslationComponent>()->SetTranslation(glm::vec3(0, 2.3, 0));

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
				AddComponent<PointLightComponent>(new PointLightComponent(colours[i], 25.0f)));
		}

		BaseApplication::Get().GetScene()->LoadAssets();
	}

	void DeferredLights::Update()
	{
		int index = 0;
		float degrees = 0.f;
		m_LightTime += Application::Get().GetDeltaTime();
		for (GameObject* obj : Application::Get().GetScene()->GetObjects())
		{
			if (PointLightComponent* lightComp = obj->GetComponent<PointLightComponent>())
			{
				TranslationComponent* comp = obj->GetComponent<TranslationComponent>();
				float radians = (glm::radians(degrees)) + (float)m_LightTime;
				float x = 5 * cos(radians);
				float z = 5 * sin(radians);

				comp->SetTranslation(glm::vec3(x, 0.f, z));
				index++;
				degrees += 60.f;
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

		BaseApplication::Get().GetScene()->GetObjects().clear();
		BaseApplication::Get().GetRenderer()->OnModelRemovedFromScene();
	}

	void DeferredLights::OnGui()
	{
		ImGui::Text("Deferred Lights");
	}

}