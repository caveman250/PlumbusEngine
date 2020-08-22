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
#include "components/LightComponent.h"

#include "renderer/vk/VulkanRenderer.h"

//REMOVE THIS
#include "renderer/vk/shader_compiler/ShaderCompiler.h"

namespace plumbus
{

	BaseApplication* BaseApplication::s_Instance = nullptr;

	BaseApplication::BaseApplication() 
		: m_Scene(nullptr)
		, m_lastUpdateTime(glfwGetTime())
		, m_Renderer(new vk::VulkanRenderer())
		, m_AppName("PlumbusEngine")
	{

	}

	void BaseApplication::Run()
	{
		vk::shaders::ShaderCompiler compiler;
		compiler.Compile("shaders/shader.vert");

		m_Renderer->Init(m_AppName);

		PL_ASSERT(m_Scene != nullptr);

		InitScene();
		MainLoop();
		Cleanup();
		m_Renderer->Cleanup();
	}
	void BaseApplication::InitScene()
	{
		//probs have to do more here
		m_Scene->Init();
	}

	void BaseApplication::SetScene(Scene* scene)
	{
		PL_ASSERT(!m_Scene || !m_Scene->IsInitialised());
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

	void BaseApplication::Cleanup()
	{
		
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