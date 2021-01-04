#include "plumbus.h"
#include "Scene.h"
#include "renderer/vk/Mesh.h"
#include "components/GameComponent.h"
#include "components/ModelComponent.h"
#include "GameObject.h"
#include "BaseApplication.h"
#include "renderer/vk/VulkanRenderer.h"

namespace plumbus
{
	Scene::Scene()
	{

	}

	void Scene::Init()
	{
		m_Camera.Init();
		m_Initialised = true;
	}

	void Scene::Shutdown()
	{
		m_Initialised = false;
	}

	bool Scene::IsInitialised()
	{
		return m_Initialised;
	}

	void Scene::AddGameObject(GameObject* obj)
	{
		m_GameObjects.push_back(obj);
	}

	void Scene::OnUpdate()
	{
		m_Camera.OnUpdate();
		for (GameObject* obj : m_GameObjects)
			obj->OnUpdate(this);
	}

	void Scene::ClearObjects()
	{
		for (GameObject* obj : m_GameObjects)
		{
			delete obj;
		}

		m_GameObjects.clear();
	}

	void Scene::LoadAssets()
	{
		for (GameObject* obj : m_GameObjects)
		{
			if (components::ModelComponent* component = obj->GetComponent<components::ModelComponent>())
			{
				component->LoadModel();
			}
		}
		for (GameObject* obj : m_GameObjects)
		{
			obj->Init();
		}
		for (GameObject* obj : m_GameObjects)
		{
			obj->PostInit();
		}
		
	}
}
