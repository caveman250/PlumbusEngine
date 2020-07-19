#pragma once

#include "plumbus.h"

namespace plumbus::base
{
	class Renderer;
}

namespace plumbus
{
	class Scene;
	class BaseApplication
	{
	public:
		BaseApplication();

		static void CreateInstance() { PL_ASSERT(false); /*create your own application class.*/ }
		static BaseApplication& Get() { return *s_Instance; }
		void Run();
		double GetDeltaTime() { return m_DeltaTime; }

		template <typename T>
		void CreateScene();
		void SetScene(Scene* scene);

		Scene* GetScene() { return m_Scene; }
		base::Renderer* GetRenderer() { return m_Renderer; }
		void MainLoop();
		virtual void Cleanup();

		virtual void OnGui();

		bool m_GameWindowFocused = false;

	protected:
		void InitScene();
		void UpdateScene();
		bool IsValidToReplaceCurrentScene();

		static BaseApplication* s_Instance;
		Scene* m_Scene;
		double m_DeltaTime = 0;
		double m_lastUpdateTime;

		base::Renderer* m_Renderer;
	};

	template <typename T>
	void BaseApplication::CreateScene()
	{
		PL_ASSERT(IsValidToReplaceCurrentScene());
		m_Scene = new T();
	}
}
