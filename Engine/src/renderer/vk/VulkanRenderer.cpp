#include "plumbus.h"

#include "renderer/vk/VulkanRenderer.h"
#include "Helpers.h"
#include "renderer/vk/ImageHelpers.h"
#include "GameObject.h"
#include "components/GameComponent.h"
#include "components/ModelComponent.h"
#include "renderer/vk/Mesh.h"
#include "components/TranslationComponent.h"
#include "Camera.h"
#include "components/LightComponent.h"
#include "imgui_impl/ImGuiImpl.h"
#include "Scene.h"
#include "Instance.h"

#if PL_PLATFORM_LINUX
#include <gtk/gtk.h>
#endif
#include <glslang/StandAlone/ResourceLimits.h>

#include "CommandBuffer.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "DescriptorPool.h"

#include "SPIRV-Cross/spirv_cpp.hpp"
#include "PipelineCache.h"
#include "PipelineLayout.h"
#include "MaterialInstance.h"

#include "glslang/glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/SPIRV/Logger.h"
#include "glslang/SPIRV/SpvTools.h"
#include "glslang/StandAlone/DirStackFileIncluder.h"
#include "shader_compiler/ShaderCompiler.h"
#include "shader_compiler/ShaderSettings.h"
#include "ShadowManager.h"
#include "ShadowDirectional.h"
#include "ShadowOmniDirectional.h"

static uint32_t s_Width, s_Height;

#if !PL_DIST
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* layerPrefix,
    const char* msg,
    void* userData)
{
    PL_ASSERT(false, "validation layer: %s", msg);

    return VK_FALSE;
}

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr)
    {
        func(instance, callback, pAllocator);
    }
}
#endif

extern int app_argc;
extern char** app_argv;

namespace plumbus::vk
{
    void VulkanRenderer::InitVulkan()
    {
        m_Instance = Instance::CreateInstance("PlumbusEngine", 1, GetRequiredValidationLayers(), GetRequiredInstanceExtensions());
#if !PL_DIST
        SetupDebugCallback();
#endif
		m_Window->CreateSurface();
        m_Device = Device::CreateDevice();
        m_PipelineCache = PipelineCache::CreatePipelineCache();
        m_SwapChain = SwapChain::CreateSwapChain();

        GenerateFullscreenQuad();

        std::vector<FrameBuffer::FrameBufferAttachmentInfo> offscreenAttachmentInfo =
        {
            FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R16G16B16A16_SFLOAT, FrameBuffer::FrameBufferAttachmentType::Colour, "position"),
            FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R16G16B16A16_SFLOAT, FrameBuffer::FrameBufferAttachmentType::Colour, "normal"),
            FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R8G8B8A8_UNORM, FrameBuffer::FrameBufferAttachmentType::Colour, "colour"),
            FrameBuffer::FrameBufferAttachmentInfo(GetDepthFormat(), FrameBuffer::FrameBufferAttachmentType::Depth, "depth")
        };

        m_DeferredFrameBuffer = FrameBuffer::CreateFrameBuffer(m_SwapChain->GetExtents().width, m_SwapChain->GetExtents().height, offscreenAttachmentInfo);


#if ENABLE_IMGUI
		std::vector<FrameBuffer::FrameBufferAttachmentInfo> outputAttachmentInfo =
		{
			FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R8G8B8A8_UNORM, FrameBuffer::FrameBufferAttachmentType::Colour, "colour"),
			FrameBuffer::FrameBufferAttachmentInfo(GetDepthFormat(), FrameBuffer::FrameBufferAttachmentType::Depth, "depth")
		};

        m_DeferredOutputFrameBuffer = FrameBuffer::CreateFrameBuffer(m_SwapChain->GetExtents().width, m_SwapChain->GetExtents().height, outputAttachmentInfo);
#endif

        m_DescriptorPool = DescriptorPool::CreateDescriptorPool(100, 100, 100);

        CreateLightsUniformBuffers();

#if ENABLE_IMGUI
        SetupImGui();
#endif
    }

#if !PL_DIST
    void VulkanRenderer::SetupDebugCallback()
    {
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = debugCallback;

        CHECK_VK_RESULT(CreateDebugReportCallbackEXT(m_Instance->GetVulkanInstance(), &createInfo, nullptr, &m_Callback));
    }
#endif


    bool VulkanRenderer::WindowShouldClose()
    {
		return static_cast<vk::Window*>(m_Window)->ShouldClose();
    }

    void VulkanRenderer::DrawFrame()
    {
        // keep in list so pointers retain their target until the present call.
        // the last element is always the next semaphore to use.
        std::vector<VkSemaphore_T*> activeSemaphores = { m_SwapChain->GetImageAvailableSemaphore() };

        UpdateOutputMaterial();
        UpdateLightsUniformBuffer();

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_Device->GetVulkanDevice(), m_SwapChain->GetVulkanSwapChain(), std::numeric_limits<uint64_t>::max(), activeSemaphores.back(), VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            Log::Fatal("failed to acquire swap chain image!");
        }

    	std::vector<ShadowDirectional*> dirShadows = ShadowManager::Get()->GetDirectionalShadows();
        std::vector<DescriptorSet::TextureUniform> dirShadowTextures;
    	for (int i = 0; i < dirShadows.size(); ++i)
    	{
            dirShadows[i]->BuildCommandBuffer();
            dirShadows[i]->Render(activeSemaphores.back());
    		dirShadowTextures.push_back({ dirShadows[i]->GetFrameBuffer()->GetSampler(), dirShadows[i]->GetFrameBuffer()->GetAttachment("depth")->m_ImageView });
    		activeSemaphores.push_back(dirShadows[i]->GetSemaphore());
    	}

    	if (!dirShadowTextures.empty() && ShadowManager::Get()->ShadowTexturesOutOfDate())
    	{
            m_DeferredOutputMaterialInstance->SetTextureUniform("samplerDirShadows", dirShadowTextures, true);
        }

        std::vector<ShadowOmniDirectional*> omniDirShadows = ShadowManager::Get()->GetOmniDirectionalShadows();
        std::vector<DescriptorSet::TextureUniform> omniDirShadowTextures;
        for (int i = 0; i < omniDirShadows.size(); ++i)
        {
            omniDirShadows[i]->UpdateUniformBuffer();

            static bool hasRecorded = false;
            if (!hasRecorded)
            {
                omniDirShadows[i]->GetCommandBuffer()->BeginRecording();
                for (int j = 0; j < omniDirShadows[i]->GetRenderPassCount(); ++j)
                {
                    omniDirShadows[i]->BuildCommandBuffer(j);
                }
                omniDirShadows[i]->GetCommandBuffer()->EndRecording();
            }

            omniDirShadows[i]->Render(activeSemaphores.back());
            omniDirShadowTextures.push_back({ omniDirShadows[i]->GetCubeMap().m_TextureSampler, omniDirShadows[i]->GetCubeMap().m_ImageView });
            activeSemaphores.push_back(omniDirShadows[i]->GetSemaphore());
        }

        if (!omniDirShadowTextures.empty() && ShadowManager::Get()->ShadowTexturesOutOfDate())
        {
            static bool hasUploaded = false;
            if(!hasUploaded)
            {
                m_DeferredOutputMaterialInstance->SetTextureUniform("samplerOmniDirShadows", omniDirShadowTextures, false);
                hasUploaded = true;
            }
        }
    	
        BuildPresentCommandBuffer(imageIndex);
        BuildDefferedCommandBuffer();
#if ENABLE_IMGUI
        BuildDeferredOutputCommandBuffer();
#endif

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        VkSemaphore waitSemaphores[] = { activeSemaphores.back() };
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_DeferredCommandBuffer->GetVulkanCommandBuffer();

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_DeferredSemaphore;
        activeSemaphores.push_back(m_DeferredSemaphore);
        CHECK_VK_RESULT(vkQueueSubmit(GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

#if ENABLE_IMGUI
        VkSemaphore imguiWaitSemaphores[] = { activeSemaphores.back() };
        submitInfo.pWaitSemaphores = imguiWaitSemaphores;
        submitInfo.pSignalSemaphores = &m_DeferredOutputSemaphore;
        activeSemaphores.push_back(m_DeferredOutputSemaphore);

        submitInfo.pCommandBuffers = &m_DeferredOutputCommandBuffer->GetVulkanCommandBuffer();
        CHECK_VK_RESULT(vkQueueSubmit(GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
#endif
        VkSemaphore presentWaitSemaphores[] = { activeSemaphores.back() };
        submitInfo.pWaitSemaphores = presentWaitSemaphores;
        submitInfo.pSignalSemaphores = &m_SwapChain->GetRenderFinishedSemaphore();

        submitInfo.pCommandBuffers = &m_SwapChain->GetCommandBuffer(imageIndex)->GetVulkanCommandBuffer();
        CHECK_VK_RESULT(vkQueueSubmit(GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
        activeSemaphores.push_back(m_SwapChain->GetRenderFinishedSemaphore());

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &activeSemaphores.back();

        VkSwapchainKHR swapChains[] = { m_SwapChain->GetVulkanSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        result = vkQueuePresentKHR(GetDevice()->GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            Log::Fatal("failed to present swap chain image!");
        }

		vkQueueWaitIdle(GetDevice()->GetPresentQueue());
    }

    void VulkanRenderer::AwaitIdle()
    {
        vkDeviceWaitIdle(m_Device->GetVulkanDevice());
    }

    void VulkanRenderer::Init(std::string appName)
    {
		m_Window = new vk::Window();
		m_Window->Init(s_Width, s_Height, appName);
#if PL_PLATFORM_LINUX
        gtk_init(&::app_argc, &::app_argv);
#endif
        InitVulkan();
    }

    void VulkanRenderer::Cleanup()
    {
    	ShadowManager::Destroy();
    	
        m_DeferredFrameBuffer.reset();
#if ENABLE_IMGUI
        m_DeferredOutputFrameBuffer.reset();
#endif
        for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
        {
            if (ModelComponent* modelComp = obj->GetComponent<ModelComponent>())
            {
                modelComp->Cleanup();
            }
        }

        m_FullscreenQuad.Cleanup();
        m_ViewPosVulkanBuffer.Cleanup();
        m_DirLightsVulkanBuffer.Cleanup();
        m_PointLightsVulkanBuffer.Cleanup();

        m_DeferredCommandBuffer.reset();
#if ENABLE_IMGUI
        delete m_ImGui;
        m_DeferredOutputCommandBuffer.reset();
#endif

        m_SwapChain->Cleanup();
        m_SwapChain.reset();

        m_DeferredOutputMaterialInstance.reset();
        m_DeferredOutputMaterial.reset();

        m_DescriptorPool.reset();

        for (auto& shaderModule : m_ShaderModules)
        {
            vkDestroyShaderModule(m_Device->GetVulkanDevice(), shaderModule, nullptr);
        }

        m_PipelineCache.reset();
        vkDestroyCommandPool(m_Device->GetVulkanDevice(), m_Device->GetCommandPool(), nullptr);


            vkDestroySemaphore(m_Device->GetVulkanDevice(), m_DeferredSemaphore, nullptr);

#if ENABLE_IMGUI

            vkDestroySemaphore(m_Device->GetVulkanDevice(), m_DeferredOutputSemaphore, nullptr);
      #endif

      m_Device.reset();

#if !PL_DIST
        DestroyDebugReportCallbackEXT(m_Instance->GetVulkanInstance(), m_Callback, nullptr);
#endif
        vkDestroySurfaceKHR(m_Instance->GetVulkanInstance(), m_Window->GetSurface(), nullptr);
        
        m_Instance->Destroy();
    }

    VkFormat VulkanRenderer::GetDepthFormat()
    {
        return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    VkFormat VulkanRenderer::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_Device->GetPhysicalDevice(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        Log::Fatal("failed to find supported format!");
        return VK_FORMAT_MAX_ENUM;
    }

    std::vector<const char*> VulkanRenderer::GetRequiredInstanceExtensions()
    {
#if PL_PLATFORM_ANDROID
        std::vector<const char*> extensions;
        extensions.push_back("VK_KHR_surface");
        extensions.push_back("VK_KHR_android_surface");
#else
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#endif
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

        return extensions;
    }

    void VulkanRenderer::GenerateFullscreenQuad()
    {
        struct Vert {
            glm::vec3 pos;
            glm::vec2 uv;
        	float dummyFloat = 0.f;
        };

        std::vector<Vert> vertexBuffer;

        vertexBuffer.push_back({ { 1.0f, 1.0f ,0.0f},{ 1.0f, 1.0f }});
        vertexBuffer.push_back({ { -1.0f, 1.0f ,0.0f},{ 0.0f, 1.0f }});
        vertexBuffer.push_back({ { -1.0f, -1.0f, 0.0f},{ 0.0f, 0.0f }});
        vertexBuffer.push_back({ { 1.0f, -1.0f, 0.0f},{ 1.0f, 0.0f }});

        CHECK_VK_RESULT(m_Device->CreateBuffer(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &m_FullscreenQuad.GetVertexBuffer(),
            vertexBuffer.size() * sizeof(Vert),
            vertexBuffer.data()));


        // Setup indices
        std::vector<uint32_t> indexBuffer = { 0, 1, 2, 2, 3, 0 };

        m_FullscreenQuad.SetIndexSize((uint32_t)indexBuffer.size());

        CHECK_VK_RESULT(m_Device->CreateBuffer(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &m_FullscreenQuad.GetIndexBuffer(),
            indexBuffer.size() * sizeof(uint32_t),
            indexBuffer.data()));
    }

    void VulkanRenderer::CreateLightsUniformBuffers()
    {
        if (!m_ViewPosVulkanBuffer.IsInitialised())
        {
            CHECK_VK_RESULT(m_Device->CreateBuffer(
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    &m_ViewPosVulkanBuffer,
                    sizeof(m_ViewPos)));

            CHECK_VK_RESULT(m_ViewPosVulkanBuffer.Map());
        }

        if (m_DirectionalLights.size() > 0)
        {
            CHECK_VK_RESULT(m_Device->CreateBuffer(
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    &m_DirLightsVulkanBuffer,
                    sizeof(DirectionalLightBufferInfo) * m_DirectionalLights.size()));

            CHECK_VK_RESULT(m_DirLightsVulkanBuffer.Map());
        }

        if (m_PointLights.size() > 0)
        {
            CHECK_VK_RESULT(m_Device->CreateBuffer(
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    &m_PointLightsVulkanBuffer,
                    sizeof(PointLightBufferInfo) * m_PointLights.size()));

            CHECK_VK_RESULT(m_PointLightsVulkanBuffer.Map());
        }

        UpdateLightsUniformBuffer();
    }

    void VulkanRenderer::BuildPresentCommandBuffer(uint32_t imageIndex)
    {
#if ENABLE_IMGUI
        m_ImGui->UpdateBuffers();
#endif

        // Set target frame buffer
        m_SwapChain->GetCommandBuffer(imageIndex)->BeginRecording();
        m_SwapChain->GetCommandBuffer(imageIndex)->BeginRenderPass();

#if ENABLE_IMGUI
        m_ImGui->DrawFrame(m_SwapChain->GetCommandBuffer(imageIndex));
#else
        m_SwapChain->GetCommandBuffer(imageIndex)->SetViewport((float)m_SwapChain->GetExtents().width, (float)m_SwapChain->GetExtents().height, 0.f, 1.f);
        m_SwapChain->GetCommandBuffer(imageIndex)->SetScissor(m_SwapChain->GetExtents().width, m_SwapChain->GetExtents().height, 0, 0);

        m_DeferredOutputMaterialInstance->Bind(m_SwapChain->GetCommandBuffer(imageIndex));
        m_SwapChain->GetCommandBuffer(imageIndex)->BindVertexBuffer(m_FullscreenQuad.GetVertexBuffer());
        m_SwapChain->GetCommandBuffer(imageIndex)->BindIndexBuffer(m_FullscreenQuad.GetIndexBuffer());
        m_SwapChain->GetCommandBuffer(imageIndex)->RecordDraw(6);
#endif
        m_SwapChain->GetCommandBuffer(imageIndex)->EndRenderPass();
        m_SwapChain->GetCommandBuffer(imageIndex)->EndRecording();
    }

    void VulkanRenderer::BuildDefferedCommandBuffer()
    {
        if (m_DeferredCommandBuffer == VK_NULL_HANDLE)
        {
            m_DeferredCommandBuffer = CommandBuffer::CreateCommandBuffer();
            m_DeferredCommandBuffer->SetFrameBuffer(m_DeferredFrameBuffer);
        }

        if (m_DeferredSemaphore == VK_NULL_HANDLE)
        {
            VkSemaphoreCreateInfo semaphoreCreateInfo{};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            CHECK_VK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &m_DeferredSemaphore));

        }

        m_DeferredCommandBuffer->BeginRecording();
        m_DeferredCommandBuffer->BeginRenderPass();
        m_DeferredCommandBuffer->SetViewport((float)m_DeferredFrameBuffer->GetWidth(), (float)m_DeferredFrameBuffer->GetHeight(), 0.f, 1.f);
        m_DeferredCommandBuffer->SetScissor(m_DeferredFrameBuffer->GetWidth(), m_DeferredFrameBuffer->GetHeight(), 0, 0);

        for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
        {
            if (ModelComponent* comp = obj->GetComponent<ModelComponent>())
            {
				for (Mesh* model : comp->GetModels())
				{
                    model->Render(m_DeferredCommandBuffer);
				}
            }
        }

        m_DeferredCommandBuffer->EndRenderPass();
        m_DeferredCommandBuffer->EndRecording();
    }

#if ENABLE_IMGUI
    void VulkanRenderer::BuildDeferredOutputCommandBuffer()
    {
        if (m_DeferredOutputCommandBuffer == VK_NULL_HANDLE)
        {
            m_DeferredOutputCommandBuffer = CommandBuffer::CreateCommandBuffer();
            m_DeferredOutputCommandBuffer->SetFrameBuffer(m_DeferredOutputFrameBuffer);
        }

        if (m_DeferredOutputSemaphore == VK_NULL_HANDLE)
        {
            VkSemaphoreCreateInfo semaphoreCreateInfo{};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            CHECK_VK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &m_DeferredOutputSemaphore));
        }

        m_DeferredOutputCommandBuffer->BeginRecording();
        m_DeferredOutputCommandBuffer->BeginRenderPass();
        m_DeferredOutputCommandBuffer->SetViewport((float)m_DeferredOutputFrameBuffer->GetWidth(), (float)m_DeferredOutputFrameBuffer->GetHeight(), 0.f, 1.f);
        m_DeferredOutputCommandBuffer->SetScissor(m_DeferredOutputFrameBuffer->GetWidth(), m_DeferredOutputFrameBuffer->GetHeight(), 0, 0);

        m_DeferredOutputMaterialInstance->Bind(m_DeferredOutputCommandBuffer);
        m_DeferredOutputCommandBuffer->BindVertexBuffer(m_FullscreenQuad.GetVertexBuffer());
        m_DeferredOutputCommandBuffer->BindIndexBuffer(m_FullscreenQuad.GetIndexBuffer());
        m_DeferredOutputCommandBuffer->RecordDraw(6);
        m_DeferredOutputCommandBuffer->EndRenderPass();
        m_DeferredOutputCommandBuffer->EndRecording();
    }
#endif

    void VulkanRenderer::GetNumLights(int& numPointLights, int& numDirLights)
    {
        numPointLights = 0;
        numDirLights = 0;

        for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
        {
            if (LightComponent* lightComp = obj->GetComponent<LightComponent>())
            {
                for (Light* light : lightComp->GetLights())
                {
                    if (light->GetType() == LightType::Point)
                    {
                        numPointLights++;
                    }
                    else if (light->GetType() == LightType::Directional)
                    {
                        numDirLights++;
                    }
                }
            }
        }
    }

    void VulkanRenderer::UpdateLightsUniformBuffer()
    {
        //TODO surely there is a way of only copying the lights that have changed.
		int pointLightIndex = 0;
        int dirLightIndex = 0;
        for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
        {
            if (LightComponent* lightComp = obj->GetComponent<LightComponent>())
            {
                for (Light* light : lightComp->GetLights())
                {
                    if (light->GetType() == LightType::Point)
                    {
                        PointLight* pointLight = static_cast<PointLight*>(light);
                        if (TranslationComponent* translationComp = obj->GetComponent<TranslationComponent>())
                        {
                            m_PointLights[pointLightIndex].m_Position = glm::vec4(translationComp->GetTranslation(), 0.f);
                            m_PointLights[pointLightIndex].m_Colour = glm::vec4(pointLight->GetColour(), 1.0f);
                            m_PointLights[pointLightIndex].m_Radius = pointLight->GetRadius();
                            pointLightIndex++;
                        }
                    }
                    else if (light->GetType() == LightType::Directional)
                    {
                        DirectionalLight* directionalLight = static_cast<DirectionalLight*>(light);
                        m_DirectionalLights[dirLightIndex].m_Direction = glm::vec4(directionalLight->GetDirection(), 1);
                        m_DirectionalLights[dirLightIndex].m_Colour = glm::vec4(directionalLight->GetColour(), 1);
                        m_DirectionalLights[dirLightIndex].m_Mvp = directionalLight->GetMVP();
                        dirLightIndex++;
                    }
                }
            }
        }

        // Current view position
        m_ViewPos = glm::vec4(BaseApplication::Get().GetScene()->GetCamera()->GetPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

        memcpy(m_ViewPosVulkanBuffer.m_Mapped, &m_ViewPos, sizeof(m_ViewPos));
        if (m_DirectionalLights.size() > 0)
        {
            memcpy(m_DirLightsVulkanBuffer.m_Mapped, m_DirectionalLights.data(), sizeof(DirectionalLightBufferInfo) * m_DirectionalLights.size());
        }
        if(m_PointLights.size() > 0)
        {
            memcpy(m_PointLightsVulkanBuffer.m_Mapped, m_PointLights.data(), sizeof(PointLightBufferInfo) * m_PointLights.size());
        }
    }

#if ENABLE_IMGUI
    void VulkanRenderer::SetupImGui()
    {
        m_ImGui = new ImGUIImpl();
        m_ImGui->Init((float)s_Width, (float)s_Height);
        m_ImGui->InitResources(m_SwapChain->GetRenderPass(), GetDevice()->GetGraphicsQueue());
    }
#endif

    VkPipelineShaderStageCreateInfo VulkanRenderer::LoadShader(std::string fileName, VkShaderStageFlagBits stage, shaders::ShaderSettings settings, ShaderReflectionObject& shaderReflection)
    {
        std::string glslText = Helpers::ReadTextFile(fileName);

    	//apply shader settings
    	Log::Info("Applying shader settings: %s", fileName.c_str());
		glslText = shaders::ShaderCompiler::ApplyShaderSettings(glslText, settings);

    	std::vector<unsigned int> SpirV;
    	Log::Info("Compiling Shader: %s", fileName.c_str());
		if(!shaders::ShaderCompiler::CompileShader(glslText, stage, SpirV))
		{
			Log::Error("Failed to compile shader %s", fileName.c_str());
		}
    	else
    	{
    		Log::Info("Compile success: %s", fileName.c_str());
    	}

        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = stage;
        shaderStage.module = CreateShaderModule(SpirV);
        shaderStage.pName = "main"; // todo : make param
        PL_ASSERT(shaderStage.module != VK_NULL_HANDLE);
        m_ShaderModules.push_back(shaderStage.module);

        spirv_cross::Compiler spirv(reinterpret_cast<uint32_t*>(SpirV.data()), SpirV.size());
        spirv_cross::ShaderResources resources = spirv.get_shader_resources();

		for (auto& resource : resources.push_constant_buffers)
		{
			uint32_t id = resource.id;
			uint32_t size = 0;
			for(auto& bufferRange : spirv.get_active_buffer_ranges(id))
			{
				size += bufferRange.range;
			}
		
			PushConstant pushConstant;
			pushConstant.m_Offset = spirv.get_decoration(resource.id, spv::DecorationOffset);
			pushConstant.m_Usage = stage == VK_SHADER_STAGE_VERTEX_BIT ? PushConstantUsage::VertexShader : PushConstantUsage::FragmentShader;
			pushConstant.m_Size = size;
			shaderReflection.m_PushConstants.push_back(pushConstant);
		}
    	
        for (auto& resource : resources.sampled_images)
        {
            DescriptorBinding binding;

            binding.m_Name = resource.name;
            binding.m_Location = spirv.get_decoration(resource.id, spv::DecorationBinding);
            binding.m_Type = DescriptorBindingType::ImageSampler;
            binding.m_Usage = stage == VK_SHADER_STAGE_VERTEX_BIT ? DescriptorBindingUsage::VertexShader : DescriptorBindingUsage::FragmentShader;
            binding.m_Count = spirv.get_type(resource.type_id).array[0];
            if (binding.m_Count == 0)
            {
                binding.m_Count = 1;
            }
            shaderReflection.m_Bindings.push_back(binding);
        }

        for (auto &resource : resources.uniform_buffers)
        {
            DescriptorBinding binding;

            binding.m_Name = resource.name;
            binding.m_Location = spirv.get_decoration(resource.id, spv::DecorationBinding);
            binding.m_Type = DescriptorBindingType::UniformBuffer;
            binding.m_Usage = stage == VK_SHADER_STAGE_VERTEX_BIT ? DescriptorBindingUsage::VertexShader : DescriptorBindingUsage::FragmentShader;
            binding.m_Count = spirv.get_type(resource.type_id).array[0];
            if (binding.m_Count == 0)
            {
                binding.m_Count = 1;
            }
            shaderReflection.m_Bindings.push_back(binding);
        }

    	auto getResourceTypeSize = [](spirv_cross::SPIRType type) 
		{
    		unsigned vecsize = type.vecsize;
    		unsigned columns = type.columns;
    		size_t component_size = type.width / 8;
    		return vecsize * component_size * columns;
    			
		};
    	for (auto& resource : resources.stage_inputs)
    	{    		
    		StageInput stageInput;
			stageInput.m_Location = spirv.get_decoration(resource.id, spv::DecorationLocation);
			stageInput.m_Binding = spirv.get_decoration(resource.id, spv::DecorationBinding);
    		stageInput.m_Size = getResourceTypeSize(spirv.get_type(resource.type_id));

    		switch (spirv.get_type(resource.type_id).basetype)
    		{
				case spirv_cross::SPIRType::Int:
					stageInput.m_Format = VK_FORMAT_R32G32B32A32_SINT;
    				break;
				case spirv_cross::SPIRType::UInt:
					stageInput.m_Format = VK_FORMAT_R32G32B32A32_UINT;
    				break;
				case spirv_cross::SPIRType::Float:
					stageInput.m_Format = VK_FORMAT_R32G32B32_SFLOAT;
					break;
				default: ;
			}

    		if (stage == VK_SHADER_STAGE_VERTEX_BIT)
    		{
    			shaderReflection.m_VertexStageInputs.push_back(stageInput);
    		}
    		else
    		{
    			shaderReflection.m_FragmentStageInputs.push_back(stageInput);
    		}
    	}

    	std::sort(shaderReflection.m_VertexStageInputs.begin(), shaderReflection.m_VertexStageInputs.end(), [](const StageInput& lhs, const StageInput& rhs){ return lhs.m_Location < rhs.m_Location;});
    	std::sort(shaderReflection.m_FragmentStageInputs.begin(), shaderReflection.m_FragmentStageInputs.end(), [](const StageInput& lhs, const StageInput& rhs){ return lhs.m_Location < rhs.m_Location;});

        //get outputs for material
        for (auto& resource : resources.stage_outputs)
        {
        	if (stage == VK_SHADER_STAGE_VERTEX_BIT)
        	{
        		shaderReflection.m_VertexStageOutputCount++;
        	}
        	else
        	{
        		shaderReflection.m_FragmentStageOutputCount++;
        	}
        }

        return shaderStage;
    }

    VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<unsigned int>& code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * (sizeof(unsigned int) / sizeof(char));
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_Device->GetVulkanDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            Log::Fatal("failed to create shader module!");
        }

		return shaderModule;
	}

	std::vector<const char*> VulkanRenderer::GetRequiredDeviceExtensions()
	{
        return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	}

	plumbus::vk::VulkanRenderer* VulkanRenderer::Get()
	{
        return static_cast<VulkanRenderer*>(BaseApplication::Get().GetRenderer());
	}

	std::vector<const char*> VulkanRenderer::GetRequiredValidationLayers()
	{
#if !PL_DIST
#if PL_PLATFORM_ANDROID
        return
        {
            "VK_LAYER_LUNARG_core_validation",
        };
#else
		return 		
		{
		    "VK_LAYER_KHRONOS_validation",
            "VK_LAYER_RENDERDOC_Capture"
		};
#endif
#else   
        return 		
		{
		};
#endif
	}

	void VulkanRenderer::UpdateOutputMaterial()
	{
        bool outputMaterialNeedsRebuild = !m_DeferredOutputMaterial.get();
        int numDirShadows = ShadowManager::Get()->GetDirectionalShadows().size();
        int numOmniDirShadows = ShadowManager::Get()->GetOmniDirectionalShadows().size();
        int numPointLights, numDirLights;
        GetNumLights(numPointLights, numDirLights);

        if (numDirShadows != m_CachedDirShadowCount ||
            numOmniDirShadows != m_CachedOmniDirShadowCount ||
            numPointLights != m_PointLights.size() ||
            numDirLights != m_DirectionalLights.size())
        {
            m_CachedDirShadowCount = numDirShadows;
            m_CachedOmniDirShadowCount = numOmniDirShadows;
            outputMaterialNeedsRebuild = true;
        }

        if (!outputMaterialNeedsRebuild)
        {
            return;
        }

        m_DeferredOutputMaterialInstance.reset();
        m_DeferredOutputMaterial.reset();

#if ENABLE_IMGUI
        m_DeferredOutputMaterial = std::make_shared<Material>("shaders/deferred.vert", "shaders/deferred.frag", m_DeferredOutputFrameBuffer->GetRenderPass());
#else
        m_DeferredOutputMaterial = std::make_shared<Material>("shaders/deferred.vert", "shaders/deferred.frag", m_SwapChain->GetRenderPass());
#endif

        shaders::ShaderSettings& settings = m_DeferredOutputMaterial->GetShaderSettings();
        settings.SetValue("NUM_DIR_SHADOWS", numDirShadows);
        settings.SetValue("NUM_OMNIDIR_SHADOWS", numOmniDirShadows);
        settings.SetValue("NUM_DIR_LIGHTS", numDirLights);
        settings.SetValue("NUM_POINT_LIGHTS", numPointLights);

        bool lightsChanged = false;
        if (m_PointLights.size() != numPointLights)
        {
            m_PointLights.resize(numPointLights);
            lightsChanged = true;
        }
        if (m_DirectionalLights.size() != numDirLights)
        {
            m_DirectionalLights.resize(numDirLights);
            lightsChanged = true;
        }

        if (lightsChanged)
        {
            m_PointLightsVulkanBuffer.Cleanup();
            m_DirLightsVulkanBuffer.Cleanup();
            CreateLightsUniformBuffers();
        }

        m_DeferredOutputMaterial->Setup();

        m_DeferredOutputMaterialInstance = MaterialInstance::CreateMaterialInstance(m_DeferredOutputMaterial);
        m_DeferredOutputMaterialInstance->SetTextureUniform("samplerposition", {{ m_DeferredFrameBuffer->GetSampler(), m_DeferredFrameBuffer->GetAttachment("position")->m_ImageView }}, false);
        m_DeferredOutputMaterialInstance->SetTextureUniform("samplerNormal", {{ m_DeferredFrameBuffer->GetSampler(), m_DeferredFrameBuffer->GetAttachment("normal")->m_ImageView }}, false);
        m_DeferredOutputMaterialInstance->SetTextureUniform("samplerAlbedo", {{ m_DeferredFrameBuffer->GetSampler(), m_DeferredFrameBuffer->GetAttachment("colour")->m_ImageView }}, false);
        m_DeferredOutputMaterialInstance->SetBufferUniform("ViewPos", &m_ViewPosVulkanBuffer);
        if (m_PointLights.size() > 0)
        {
            m_DeferredOutputMaterialInstance->SetBufferUniform("PointLights", &m_PointLightsVulkanBuffer);
        }
        if (m_DirectionalLights.size() > 0)
        {
            m_DeferredOutputMaterialInstance->SetBufferUniform("DirectionalLights", &m_DirLightsVulkanBuffer);
        }
	}

	void VulkanRenderer::RecreateSwapChain()
	{
        vkDeviceWaitIdle(m_Device->GetVulkanDevice());

        m_SwapChain->Recreate();

#if ENABLE_IMGUI
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(m_SwapChain->GetExtents().width / m_Window->GetContentScaleX()), static_cast<float>(m_SwapChain->GetExtents().height / m_Window->GetContentScaleY()));
#endif
	}

}
