#include "Application.h"

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <set>
#include <algorithm>
#include "Helpers.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <cstring>

#include "GameObject.h"
#include "components/Component.h"
#include "components/ModelComponent.h"
#include "renderer/vk/Model.h"
#include "components/TranslationComponent.h"
#include "Camera.h"
#include "components/PointLightComponent.h"
#include "imgui_impl/ImGuiImpl.h"
#include "imgui_impl/Log.h"

#include "renderer/base/Renderer.h"

#ifdef VULKAN_RENDERER
    #include "renderer/vk/VulkanRenderer.h"
#endif



Application* Application::m_Instance = nullptr;

Application::Application() :
	m_Scene(new Scene()),
	m_lastUpdateTime(glfwGetTime())
{
#if VULKAN_RENDERER
    m_Renderer = new VulkanRenderer();
#elif METAL_RENDERER
    //TODO
#endif

}

void Application::Run()
{
	m_Renderer->Init();
	InitScene();
	MainLoop();
    m_Renderer->Cleanup();
}



void Application::InitScene()
{
	//probs have to do more here
	m_Scene->Init();
}


void Application::MainLoop()
{
	while (!m_Renderer->WindowShouldClose())
	{
		glfwPollEvents();

		double currTime = glfwGetTime();
		m_DeltaTime = currTime - m_lastUpdateTime;
		m_lastUpdateTime = currTime;

		UpdateScene();
		m_Renderer->DrawFrame();

        //if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE))
        //	glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
	}

    m_Renderer->AwaitIdle();
}

void Application::UpdateScene()
{
	m_Scene->OnUpdate();
}


