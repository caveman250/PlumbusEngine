#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
	class VulkanRenderer;
}

namespace plumbus
{
	class Scene;
	class BaseApplication
	{
	public:
		BaseApplication();

		static void CreateInstance() { s_Instance = new BaseApplication(); }
		static BaseApplication& Get() { return *s_Instance; }
		void Run();
		double GetDeltaTime() { return m_DeltaTime; }

		template <typename T>
		void CreateScene();
		void SetScene(Scene* scene);

		Scene* GetScene() { return m_Scene; }
		vk::VulkanRenderer* GetRenderer() { return m_Renderer; }
		void MainLoop();
		virtual void Cleanup();

		void SetAppName(std::string name) { m_AppName = name; }

		bool m_GameWindowFocused = false;

	protected:
		void InitScene();
		void UpdateScene();
		bool IsValidToReplaceCurrentScene();

		static BaseApplication* s_Instance;
		Scene* m_Scene;
		double m_DeltaTime = 0;
		double m_LastUpdateTime;

		vk::VulkanRenderer* m_Renderer;
		std::string m_AppName;
	};

	template <typename T>
	void BaseApplication::CreateScene()
	{
		PL_ASSERT(IsValidToReplaceCurrentScene());
		m_Scene = new T();
	}
}

#if DLL_EXPORTS
#if defined PL_PLATFORM_WINDOWS
        #define LIB_API(RetType) extern "C" __declspec(dllexport) RetType
    #else
        #define LIB_API(RetType) extern "C" RetType __attribute__((visibility("default")))
    #endif
#else
#if defined PL_PLATFORM_WINDOWS
#define LIB_API(RetType) extern "C" __declspec(dllimport) RetType
#else
#define LIB_API(RetType) extern "C" RetType
#endif
#endif

LIB_API(void) RunApplication();