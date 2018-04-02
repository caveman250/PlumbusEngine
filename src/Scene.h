#pragma once
#include "Application.h"
#include "ModelManager.h"
#include "Camera.h"

class GameObject;
class Scene
{
public:
	Scene();
	Camera* GetCamera() { return &m_Camera; }

	void Init();
	void OnUpdate();

	void AddGameObject(GameObject* obj) { m_GameObjects.push_back(obj); }
	std::vector<GameObject*> GetObjects() { return m_GameObjects; }
	void LoadModels();

private:
	Camera m_Camera;
	std::vector<GameObject*> m_GameObjects;
};