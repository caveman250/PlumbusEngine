#pragma once

#include <vector>
#include <array>
#include "Scene.h"

class Scene;
namespace base
{
	class Renderer;
}

class Application
{
public:
	Application();

	static void CreateInstance() { m_Instance = new Application(); }
	static Application& Get() { return *m_Instance; }
	void Run();
	double GetDeltaTime() { return m_DeltaTime; }
	Scene* GetScene() { return m_Scene; }
    base::Renderer* GetRenderer() { return m_Renderer; }

	double m_LightTime = 0;
	bool m_GameFocued = false;

private:

	void InitScene();
	void MainLoop();
	void UpdateScene();

	static Application* m_Instance;
	Scene* m_Scene;
	double m_DeltaTime = 0;
	double m_lastUpdateTime;

    base::Renderer* m_Renderer;

};
