#include "plumbus.h"

#include "BaseApplication.h"
#include "Helpers.h"
#include "GameObject.h"
#include "components/GameComponent.h"
#include "components/ModelComponent.h"
#include "renderer/vk/Mesh.h"
#include "components/TranslationComponent.h"
#include "Camera.h"
#include "Scene.h"
#include "components/PointLightComponent.h"

#include "renderer/base/Renderer.h"

#if VULKAN_RENDERER
    #include "renderer/vk/VulkanRenderer.h"
#elif METAL_RENDERER
    #include "renderer/mtl/MetalRenderer.h"
#endif

namespace plumbus
{

	BaseApplication* BaseApplication::s_Instance = nullptr;

	BaseApplication::BaseApplication() 
		: m_Scene(nullptr)
		, m_lastUpdateTime(glfwGetTime())
	{
#if VULKAN_RENDERER
		m_Renderer = new vk::VulkanRenderer();
#elif METAL_RENDERER
		m_Renderer = new mtl::MetalRenderer();
#endif

	}

	void BaseApplication::Run()
	{
		PLUMBUS_ASSERT(m_Scene != nullptr);

		m_Renderer->Init();
		InitScene();
		MainLoop();
		m_Renderer->Cleanup();
	}
	void BaseApplication::InitScene()
	{
		//probs have to do more here
		m_Scene->Init();
	}

	void BaseApplication::SetScene(Scene* scene)
	{
		PLUMBUS_ASSERT(!m_Scene || !m_Scene->IsInitialised());
		m_Scene = scene;
	}

	void BaseApplication::MainLoop()
	{
		while (!m_Renderer->WindowShouldClose())
		{
			glfwPollEvents();

			double currTime = glfwGetTime();
			m_DeltaTime = currTime - m_lastUpdateTime;
			m_lastUpdateTime = currTime;

			UpdateScene();
			m_Renderer->DrawFrame();
		}

		m_Renderer->AwaitIdle();
	}

	void BaseApplication::OnGui()
	{

	}

	void BaseApplication::UpdateScene()
	{
		m_Scene->OnUpdate();
	}

	bool BaseApplication::IsValidToReplaceCurrentScene()
	{
		return !m_Scene || !m_Scene->IsInitialised();
	}
}