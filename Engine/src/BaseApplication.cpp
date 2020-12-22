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
#if !PL_PLATFORM_ANDROID
            , m_LastUpdateTime(glfwGetTime())
#else
            , m_LastUpdateTime(0)
#endif
            , m_Renderer(new vk::VulkanRenderer()), m_AppName("PlumbusEngine")
    {
    }

    void BaseApplication::Run()
    {
        vk::shaders::ShaderCompiler compiler;
        compiler.Compile("shaders/shader.vert");
        //return;

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

#if PL_PLATFORM_ANDROID
    static double now_seconds(void)
    {
    	static constexpr double nanoSecondsToSeconds = 1000000000;
        struct timespec res;
        clock_gettime(CLOCK_MONOTONIC, &res);
        return res.tv_sec + (res.tv_nsec / nanoSecondsToSeconds);
    }
#endif


    void BaseApplication::MainLoop()
    {
        while (!m_Renderer->WindowShouldClose())
        {
#if !PL_PLATFORM_ANDROID
            glfwPollEvents();

            double currTime = glfwGetTime();
#else
            double currTime = now_seconds();
#endif

			m_DeltaTime = currTime - m_LastUpdateTime;
			m_LastUpdateTime = currTime;

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