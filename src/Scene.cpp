#include "Scene.h"
#include "ModelComponent.h"

Scene::Scene()
{

}

void Scene::Init()
{
	m_Camera.Init();
}

void Scene::OnUpdate()
{
	m_Camera.OnUpdate();
	for (GameObject* obj : m_GameObjects)
		obj->OnUpdate(this);
}

void Scene::LoadModels()
{
	for (GameObject* obj : m_GameObjects)
	{
		if (ModelComponent* component = obj->GetComponent<ModelComponent>())
		{
			component->LoadModel();
		}
	}
}
