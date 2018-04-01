#include "Scene.h"

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
}

void Scene::LoadModel(std::string modelPath, std::string texturePath)
{
	m_ModelManager.AddModel(m_ModelManager.GetModelLoader().LoadModel(modelPath, texturePath));
}

