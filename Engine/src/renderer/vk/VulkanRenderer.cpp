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
#include "CommandBuffer.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "DescriptorPool.h"

#include "SPIRV-Cross/spirv_cpp.hpp"
#include "PipelineCache.h"
#include "PipelineLayout.h"
#include "MaterialInstance.h"

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
            FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R16G16B16A16_SFLOAT, false, "position"),
            FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R16G16B16A16_SFLOAT, false, "normal"),
            FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R8G8B8A8_UNORM, false, "colour"),
            FrameBuffer::FrameBufferAttachmentInfo(GetDepthFormat(), true, "depth")
        };
        m_DeferredFrameBuffer = FrameBuffer::CreateFrameBuffer(m_SwapChain->GetExtents().width, m_SwapChain->GetExtents().height, offscreenAttachmentInfo);

#if !PL_DIST
		std::vector<FrameBuffer::FrameBufferAttachmentInfo> outputAttachmentInfo =
		{
			FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R8G8B8A8_UNORM, false, "colour"),
			FrameBuffer::FrameBufferAttachmentInfo(GetDepthFormat(), true, "depth")
		};
		m_DeferredOutputFrameBuffer = FrameBuffer::CreateFrameBuffer(m_SwapChain->GetExtents().width, m_SwapChain->GetExtents().height, outputAttachmentInfo);
#endif

        CreateLightsUniformBuffers();
        m_DescriptorPool = DescriptorPool::CreateDescriptorPool(100, 100, 100);

#if !PL_DIST
        m_DeferredOutputMaterial = std::make_shared<Material>("shaders/deferred.vert.spv", "shaders/deferred.frag.spv", m_DeferredOutputFrameBuffer->GetRenderPass());
#else
        m_DeferredOutputMaterial = std::make_shared<Material>("shaders/deferred.vert.spv", "shaders/deferred.frag.spv", m_SwapChain->GetRenderPass());
#endif
        m_DeferredOutputMaterial->Setup(VertexLayout());

        m_DeferredOutputMaterialInstance = MaterialInstance::CreateMaterialInstance(m_DeferredOutputMaterial);
        const FrameBuffer::FrameBufferAttachment& positionAttachment = m_DeferredFrameBuffer->GetAttachment("position");
        m_DeferredOutputMaterialInstance->SetTextureUniform("samplerposition", m_DeferredFrameBuffer->GetSampler(), m_DeferredFrameBuffer->GetAttachment("position").m_ImageView);
        m_DeferredOutputMaterialInstance->SetTextureUniform("samplerNormal", m_DeferredFrameBuffer->GetSampler(), m_DeferredFrameBuffer->GetAttachment("normal").m_ImageView);
        m_DeferredOutputMaterialInstance->SetTextureUniform("samplerAlbedo", m_DeferredFrameBuffer->GetSampler(), m_DeferredFrameBuffer->GetAttachment("colour").m_ImageView);
        m_DeferredOutputMaterialInstance->SetBufferUniform("UBO", &m_LightsVulkanBuffer);

        BuildDefferedCommandBuffer();
#if !PL_DIST
        SetupImGui();
        BuildDeferredOutputCommandBuffer();
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

    void VulkanRenderer::DrawFrame()
    {
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_Device->GetVulkanDevice(), m_SwapChain->GetVulkanSwapChain(), std::numeric_limits<uint64_t>::max(), m_SwapChain->GetImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            Log::Fatal("failed to acquire swap chain image!");
        }

        BuildPresentCommandBuffer(imageIndex);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_SwapChain->GetImageAvailableSemaphore() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_DeferredCommandBuffer->GetVulkanCommandBuffer();

        VkSemaphore signalSemaphores[] = { m_DeferredSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        CHECK_VK_RESULT(vkQueueSubmit(GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

#if !PL_DIST
        submitInfo.pWaitSemaphores = &m_DeferredSemaphore;
        submitInfo.pSignalSemaphores = &m_DeferredOutputSemaphore;

        submitInfo.pCommandBuffers = &m_DeferredOutputCommandBuffer->GetVulkanCommandBuffer();
        CHECK_VK_RESULT(vkQueueSubmit(GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

        submitInfo.pWaitSemaphores = &m_DeferredOutputSemaphore;
#else
        submitInfo.pWaitSemaphores = &m_DeferredSemaphore;
#endif
        submitInfo.pSignalSemaphores = &m_SwapChain->GetRenderFinishedSemaphore();

        submitInfo.pCommandBuffers = &m_SwapChain->GetCommandBuffer(imageIndex)->GetVulkanCommandBuffer();
        CHECK_VK_RESULT(vkQueueSubmit(GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_SwapChain->GetRenderFinishedSemaphore();

        VkSwapchainKHR swapChains[] = { m_SwapChain->GetVulkanSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

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

        UpdateLightsUniformBuffer();
    }

    bool VulkanRenderer::WindowShouldClose()
    {
		return static_cast<vk::Window*>(m_Window)->ShouldClose();
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
        m_DeferredFrameBuffer.reset();
#if !PL_DIST
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
        m_LightsVulkanBuffer.Cleanup();

        m_DeferredCommandBuffer.reset();
#if !PL_DIST
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
#if !PL_DIST
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
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

        return extensions;
    }

    void VulkanRenderer::GenerateFullscreenQuad()
    {
        struct Vert {
            float pos[3];
            float uv[2];
            float col[3];
            float normal[3];
            float tangent[3];
        };

        std::vector<Vert> vertexBuffer;

        vertexBuffer.push_back({ { 1.0f, 1.0f, 0.0f },{ 1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f } });
        vertexBuffer.push_back({ { 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f } });
        vertexBuffer.push_back({ { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f } });
        vertexBuffer.push_back({ { 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 0.0f } });

        CHECK_VK_RESULT(m_Device->CreateBuffer(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &m_FullscreenQuad.GetVertexBuffer(),
            vertexBuffer.size() * sizeof(Vert),
            vertexBuffer.data()));


        // Setup indices
        std::vector<uint32_t> indexBuffer = { 0,1,2, 2,3,0 };

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
        // Deferred fragment shader
        CHECK_VK_RESULT(m_Device->CreateBuffer(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &m_LightsVulkanBuffer,
            sizeof(m_LightsUniformBuffer)));

        CHECK_VK_RESULT(m_LightsVulkanBuffer.Map());

        UpdateLightsUniformBuffer();
    }

    void VulkanRenderer::BuildPresentCommandBuffer(int index)
    {
#if !PL_DIST 
        m_ImGui->NewFrame();
        m_ImGui->UpdateBuffers();
#endif

        // Set target frame buffer
        m_SwapChain->GetCommandBuffer(index)->BeginRecording();
        m_SwapChain->GetCommandBuffer(index)->BeginRenderPass();

#if !PL_DIST
        m_ImGui->DrawFrame(m_SwapChain->GetCommandBuffer(index)->GetVulkanCommandBuffer());
#else
        m_SwapChain->GetCommandBuffer(index)->SetViewport((float)m_SwapChain->GetExtents().width, (float)m_SwapChain->GetExtents().height, 0.f, 1.f);
        m_SwapChain->GetCommandBuffer(index)->SetScissor(m_SwapChain->GetExtents().width, m_SwapChain->GetExtents().height, 0, 0);

        m_DeferredOutputMaterialInstance->Bind(m_SwapChain->GetCommandBuffer(index));
        m_SwapChain->GetCommandBuffer(index)->BindVertexBuffer(m_FullscreenQuad.GetVertexBuffer());
        m_SwapChain->GetCommandBuffer(index)->BindIndexBuffer(m_FullscreenQuad.GetIndexBuffer());
        m_SwapChain->GetCommandBuffer(index)->RecordDraw(6);
#endif
        m_SwapChain->GetCommandBuffer(index)->EndRenderPass();
        m_SwapChain->GetCommandBuffer(index)->EndRecording();
    }

    void VulkanRenderer::BuildDefferedCommandBuffer()
    {
        if (!m_DeferredCommandBuffer)
        {
            m_DeferredCommandBuffer = CommandBuffer::CreateCommandBuffer();
            m_DeferredCommandBuffer->SetFrameBuffer(m_DeferredFrameBuffer);
        }

        if(m_DeferredSemaphore == VK_NULL_HANDLE)
        {
            // Create a semaphore used to synchronize offscreen rendering and usage
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
                    model->Render();
				}
            }
        }

        m_DeferredCommandBuffer->EndRenderPass();
        m_DeferredCommandBuffer->EndRecording();
    }

#if !PL_DIST
    void VulkanRenderer::BuildDeferredOutputCommandBuffer()
    {
        if (!m_DeferredOutputCommandBuffer)
        {
            m_DeferredOutputCommandBuffer = CommandBuffer::CreateCommandBuffer();
            m_DeferredOutputCommandBuffer->SetFrameBuffer(m_DeferredOutputFrameBuffer);
        }

        // Create a semaphore used to synchronize offscreen rendering and usage
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        CHECK_VK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &m_DeferredOutputSemaphore));

        m_DeferredOutputCommandBuffer->BeginRecording();
        m_DeferredOutputCommandBuffer->BeginRenderPass();
        m_DeferredOutputCommandBuffer->SetViewport((float)m_SwapChain->GetExtents().width, (float)m_SwapChain->GetExtents().height, 0.f, 1.f);
        m_DeferredOutputCommandBuffer->SetScissor(m_SwapChain->GetExtents().width, m_SwapChain->GetExtents().height, 0, 0);

        m_DeferredOutputMaterialInstance->Bind(m_DeferredOutputCommandBuffer);
        m_DeferredOutputCommandBuffer->BindVertexBuffer(m_FullscreenQuad.GetVertexBuffer());
        m_DeferredOutputCommandBuffer->BindIndexBuffer(m_FullscreenQuad.GetIndexBuffer());
        m_DeferredOutputCommandBuffer->RecordDraw(6);
        m_DeferredOutputCommandBuffer->EndRenderPass();
        m_DeferredOutputCommandBuffer->EndRecording();
    }
#endif

    void VulkanRenderer::UpdateLightsUniformBuffer()
    {
        //TODO surely there is a way of only copying the lights that have changed.

        //clear the buffer, this should be smarter at some point.
        memset(&m_LightsUniformBuffer, 0, sizeof(m_LightsUniformBuffer));

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
                            m_LightsUniformBuffer.m_PointLights[pointLightIndex].m_Position = glm::vec4(translationComp->GetTranslation(), 0.f);
                            m_LightsUniformBuffer.m_PointLights[pointLightIndex].m_Colour = pointLight->GetColour();
                            m_LightsUniformBuffer.m_PointLights[pointLightIndex].m_Radius = pointLight->GetRadius();
                            pointLightIndex++;
                        }
                    }
                    else if (light->GetType() == LightType::Directional)
                    {
                        DirectionalLight* directionalLight = static_cast<DirectionalLight*>(light);
                        m_LightsUniformBuffer.m_DirectionalLights[dirLightIndex].m_Direction = directionalLight->GetDirection();
						m_LightsUniformBuffer.m_DirectionalLights[dirLightIndex].m_Colour = glm::vec4(directionalLight->GetColour(), 1);
                    }
                }
            }
        }

        // Current view position
        m_LightsUniformBuffer.m_ViewPos = glm::vec4(BaseApplication::Get().GetScene()->GetCamera()->GetPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

        memcpy(m_LightsVulkanBuffer.m_Mapped, &m_LightsUniformBuffer, sizeof(m_LightsUniformBuffer));
    }

#if !PL_DIST
    void VulkanRenderer::SetupImGui()
    {
        m_ImGui = new ImGUIImpl();
        m_ImGui->Init((float)s_Width, (float)s_Height);
        m_ImGui->InitResources(m_SwapChain->GetRenderPass(), GetDevice()->GetGraphicsQueue());
    }
#endif

    VkPipelineShaderStageCreateInfo VulkanRenderer::LoadShader(std::string fileName, VkShaderStageFlagBits stage, std::vector<DescriptorBinding>& outBindingInfo, int& numOutputs)
    {
        std::vector<char> spirvText = Helpers::ReadFile(fileName);
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = stage;
        shaderStage.module = CreateShaderModule(spirvText);
        shaderStage.pName = "main"; // todo : make param
        PL_ASSERT(shaderStage.module != VK_NULL_HANDLE);
        m_ShaderModules.push_back(shaderStage.module);

        spirv_cross::Compiler spirv(reinterpret_cast<uint32_t*>(spirvText.data()), spirvText.size() / (sizeof(uint32_t) / sizeof(char)));
        spirv_cross::ShaderResources resources = spirv.get_shader_resources();

        for (auto &resource : resources.sampled_images)
        {
            DescriptorBinding binding;

            binding.m_Name = resource.name;
            binding.m_Location = spirv.get_decoration(resource.id, spv::DecorationBinding);
            binding.m_Type = DescriptorBindingType::ImageSampler;
            binding.m_Usage = stage == VK_SHADER_STAGE_VERTEX_BIT ? DescriptorBindingUsage::VertexShader : DescriptorBindingUsage::FragmentShader; 
            outBindingInfo.push_back(binding);
        }

        for (auto &resource : resources.uniform_buffers)
        {
            DescriptorBinding binding;

            binding.m_Name = resource.name;
            binding.m_Location = spirv.get_decoration(resource.id, spv::DecorationBinding);
            binding.m_Type = DescriptorBindingType::UniformBuffer;
            binding.m_Usage = stage == VK_SHADER_STAGE_VERTEX_BIT ? DescriptorBindingUsage::VertexShader : DescriptorBindingUsage::FragmentShader;
            outBindingInfo.push_back(binding);
        }

        //get outputs for material
        for (auto& resource : resources.stage_outputs)
        {
            numOutputs++;
        }

        return shaderStage;
    }

    VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_Device->GetVulkanDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            Log::Fatal("failed to create shader module!");
        }

		return shaderModule;
	}

	void VulkanRenderer::OnModelAddedToScene()
	{
		BuildDefferedCommandBuffer();
	}

	void VulkanRenderer::OnModelRemovedFromScene()
	{
		BuildDefferedCommandBuffer();
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
		return 		
		{
			"VK_LAYER_KHRONOS_validation",
			//"VK_LAYER_RENDERDOC_Capture"
		};
#else   
        return {};
#endif
	}

	void VulkanRenderer::RecreateSwapChain()
	{
        vkDeviceWaitIdle(m_Device->GetVulkanDevice());

        m_SwapChain->Recreate();

#if !PL_DIST
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(m_SwapChain->GetExtents().width / m_Window->GetContentScaleX()), static_cast<float>(m_SwapChain->GetExtents().height / m_Window->GetContentScaleY()));
#endif
	}

}
