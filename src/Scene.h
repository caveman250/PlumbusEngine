#pragma once
#include "Application.h"
#include "ModelManager.h"
#include "Camera.h"

class Scene
{
public:
	Scene();
	ModelManager* GetModelManager() { return &m_ModelManager; }
	Camera* GetCamera() { return &m_Camera; }

	void Init();
	void OnUpdate();

	void LoadModel(std::string modelPath, std::string texturePath);

private:
	ModelManager m_ModelManager;
	Camera m_Camera;
};