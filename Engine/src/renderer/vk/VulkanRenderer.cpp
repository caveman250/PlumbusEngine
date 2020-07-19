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

#if PLUMBUS_PLATFORM_LINUX
#include <gtk/gtk.h>
#endif
#include "CommandBuffer.h"

static uint32_t s_Width, s_Height;

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

extern int app_argc;
extern char** app_argv;

namespace plumbus::vk
{
    void VulkanRenderer::InitVulkan()
    {
        m_Instance = Instance::CreateInstance("PlumbusEngine", 1, GetRequiredValidationLayers(), GetRequiredInstanceExtensions());
        SetupDebugCallback();
		GetVulkanWindow()->CreateSurface();
        m_Device = Device::CreateDevice();
        CreatePipelineCache();
        m_SwapChain = SwapChain::CreateSwapChain();

        GenerateQuads();

        std::vector<FrameBuffer::FrameBufferAttachmentInfo> offscreenAttachmentInfo =
        {
            FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R16G16B16A16_SFLOAT, false, "position"),
            FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R16G16B16A16_SFLOAT, false, "normal"),
            FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R8G8B8A8_UNORM, false, "colour"),
            FrameBuffer::FrameBufferAttachmentInfo(FindDepthFormat(), true, "depth")
        };
        m_OffscreenFrameBuffer = FrameBuffer::CreateFrameBuffer(m_SwapChain->GetExtents().width, m_SwapChain->GetExtents().height, offscreenAttachmentInfo);

		std::vector<FrameBuffer::FrameBufferAttachmentInfo> outputAttachmentInfo =
		{
			FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R8G8B8A8_UNORM, false, "colour"),
			FrameBuffer::FrameBufferAttachmentInfo(FindDepthFormat(), true, "depth")
		};
		m_OutputFrameBuffer = FrameBuffer::CreateFrameBuffer(m_SwapChain->GetExtents().width, m_SwapChain->GetExtents().height, outputAttachmentInfo);

        CreateUniformBuffers();
        CreateDescriptorSetLayout();
		CreatePipelines();
        CreateDescriptorPool();
        CreateDescriptorSet();

        SetupImGui();
        m_OutputTexture.CreateTextureSampler();
        m_OutputTexture.m_ImageView = m_OutputFrameBuffer->GetAttachment("colour").m_ImageView;

        BuildDefferedCommandBuffer();
        BuildOutputFrameBuffer();
    }

   

    void VulkanRenderer::SetupDebugCallback()
    {
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = debugCallback;

        CHECK_VK_RESULT(CreateDebugReportCallbackEXT(m_Instance->GetVulkanInstance(), &createInfo, nullptr, &m_Callback));
    }

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

        BuildImguiCommandBuffer(imageIndex);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_SwapChain->GetImageAvailableSemaphore() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_OffScreenCmdBuffer->GetVulkanCommandBuffer();

        VkSemaphore signalSemaphores[] = { m_OffscreenSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        CHECK_VK_RESULT(vkQueueSubmit(GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

        submitInfo.pWaitSemaphores = &m_OffscreenSemaphore;
        submitInfo.pSignalSemaphores = &m_OutputSemaphore;

        submitInfo.pCommandBuffers = &m_OutputCmdBuffer->GetVulkanCommandBuffer();
        CHECK_VK_RESULT(vkQueueSubmit(GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

        submitInfo.pWaitSemaphores = &m_OutputSemaphore;
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

    void VulkanRenderer::Init()
    {
		m_Window = new vk::Window();
		m_Window->Init(s_Width, s_Height);
#if PLUMBUS_PLATFORM_LINUX
        gtk_init(&::app_argc, &::app_argv);
#endif
        InitVulkan();
    }

    void VulkanRenderer::Cleanup()
    {
        m_OffscreenFrameBuffer.reset();
        m_OutputFrameBuffer.reset();

        vkDestroySampler(m_Device->GetVulkanDevice(), m_OutputTexture.m_TextureSampler, nullptr);

        vkDestroyPipeline(m_Device->GetVulkanDevice(), m_Pipelines.m_Deferred, nullptr);
        vkDestroyPipelineLayout(m_Device->GetVulkanDevice(), m_PipelineLayouts.m_Deferred, nullptr);
        vkDestroyPipeline(m_Device->GetVulkanDevice(), m_Pipelines.m_Offscreen, nullptr);
        vkDestroyPipelineLayout(m_Device->GetVulkanDevice(), m_PipelineLayouts.m_Offscreen, nullptr);
        vkDestroyPipeline(m_Device->GetVulkanDevice(), m_Pipelines.m_Output, nullptr);

        vkDestroyDescriptorSetLayout(m_Device->GetVulkanDevice(), m_DescriptorSetLayout, nullptr);

        for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
        {
            if (ModelComponent* modelComp = obj->GetComponent<ModelComponent>())
            {
                modelComp->Cleanup();
            }
        }

        m_ScreenQuad.Cleanup();

        m_UniformBuffers.m_VertFullScreen.Cleanup();
        m_UniformBuffers.m_FragLights.Cleanup();

        delete m_ImGui;

        m_OffScreenCmdBuffer.reset();
        m_OutputCmdBuffer.reset();

        m_SwapChain->Cleanup();
        m_SwapChain.reset();

        vkDestroyDescriptorPool(m_Device->GetVulkanDevice(), m_DescriptorPool, nullptr);

        for (auto& shaderModule : m_ShaderModules)
        {
            vkDestroyShaderModule(m_Device->GetVulkanDevice(), shaderModule, nullptr);
        }

        vkDestroyPipelineCache(m_Device->GetVulkanDevice(), m_PipelineCache, nullptr);
        vkDestroyCommandPool(m_Device->GetVulkanDevice(), m_Device->GetCommandPool(), nullptr);

        vkDestroySemaphore(m_Device->GetVulkanDevice(), m_OutputSemaphore, nullptr);
        vkDestroySemaphore(m_Device->GetVulkanDevice(), m_OffscreenSemaphore, nullptr);

        m_Device.reset();

        DestroyDebugReportCallbackEXT(m_Instance->GetVulkanInstance(), m_Callback, nullptr);
        vkDestroySurfaceKHR(m_Instance->GetVulkanInstance(), GetVulkanWindow()->GetSurface(), nullptr);
        
        m_Instance->Destroy();
    }

    void VulkanRenderer::OnWindowResized(GLFWwindow* window, int width, int height)
    {
        BaseApplication* app = reinterpret_cast<BaseApplication*>(glfwGetWindowUserPointer(window));
        VulkanRenderer::Get()->RecreateSwapChain();
    }

    VkFormat VulkanRenderer::FindDepthFormat()
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

    void VulkanRenderer::CreatePipelineCache()
    {
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        CHECK_VK_RESULT(vkCreatePipelineCache(m_Device->GetVulkanDevice(), &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));
    }

    void VulkanRenderer::GenerateQuads()
    {
        struct Vert {
            float pos[3];
            float uv[2];
            float col[3];
            float normal[3];
            float tangent[3];
        };

        std::vector<Vert> vertexBuffer;

        float x = 0.0f;
        float y = 0.0f;
        for (uint32_t i = 0; i < 1; i++)
        {
            // Last component of normal is used for debug display sampler index
            vertexBuffer.push_back({ { x + 1.0f, y + 1.0f, 0.0f },{ 1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, (float)i } });
            vertexBuffer.push_back({ { x,      y + 1.0f, 0.0f },{ 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, (float)i } });
            vertexBuffer.push_back({ { x,      y,      0.0f },{ 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, (float)i } });
            vertexBuffer.push_back({ { x + 1.0f, y,      0.0f },{ 1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, (float)i } });
            x += 1.0f;
            if (x > 1.0f)
            {
                x = 0.0f;
                y += 1.0f;
            }
        }

        CHECK_VK_RESULT(m_Device->CreateBuffer(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &m_ScreenQuad.GetVertexBuffer(),
            vertexBuffer.size() * sizeof(Vert),
            vertexBuffer.data()));


        // Setup indices
        std::vector<uint32_t> indexBuffer = { 0,1,2, 2,3,0 };
        for (uint32_t i = 0; i < 1; ++i)
        {
            uint32_t indices[6] = { 0,1,2, 2,3,0 };
            for (auto index : indices)
            {
                indexBuffer.push_back(i * 4 + index);
            }
        }

        m_ScreenQuad.SetIndexSize((uint32_t)indexBuffer.size());

        CHECK_VK_RESULT(m_Device->CreateBuffer(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &m_ScreenQuad.GetIndexBuffer(),
            indexBuffer.size() * sizeof(uint32_t),
            indexBuffer.data()));
    }

   

    void VulkanRenderer::InitLightsVBO()
    {
        for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
        {
            int pointLightIndex = 0;
            int dirLightIndex = 0;
            if (LightComponent* comp = obj->GetComponent<LightComponent>())
            {
                for (Light* light : comp->GetLights())
                {
                    if (light->GetType() == LightType::Point)
                    {
                        PL_ASSERT(pointLightIndex < MAX_POINT_LIGHTS, "Point light count exceeds MAX_POINT_LIGHTS");

                        PointLight* pointLight = static_cast<PointLight*>(light);

                        PointLightBufferInfo info = {};
                        info.m_Colour = pointLight->GetColour();
                        info.m_Radius = pointLight->GetRadius();

                        TranslationComponent* trans = obj->GetComponent<TranslationComponent>();
                        trans->SetTranslation(glm::vec3(5.f, 1.f, 0.f));
                        info.m_Position = glm::vec4(trans->GetTranslation(), 0.f);

                        m_LightsUBO.m_PointLights[pointLightIndex] = info;
                        ++pointLightIndex;
                    }
                    else if (light->GetType() == LightType::Directional)
                    {
						PL_ASSERT(dirLightIndex < MAX_DIRECTIONAL_LIGHTS, "Directional light count exceeds MAX_DIRECTIONAL_LIGHTS");

						DirectionalLight* directionalLight = static_cast<DirectionalLight*>(light);

                        DirectionalLightBufferInfo info = {};
                        info.m_Colour = directionalLight->GetColour();
                        info.m_Direction = directionalLight->GetDirection();

						m_LightsUBO.m_DirectionalLights[dirLightIndex] = info;
						++dirLightIndex;
                    }
                    else
                    {
                        Log::Error("VulkanRenderer::InitLightsVBO() Invalid light type. GameObject: %s", obj->GetID());
                    }
                }
            }
        }
    }

    void VulkanRenderer::CreateUniformBuffers()
    {
        // Fullscreen vertex shader
        CHECK_VK_RESULT(m_Device->CreateBuffer(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &m_UniformBuffers.m_VertFullScreen,
            sizeof(m_VertUBO)));

        // Deferred fragment shader
        CHECK_VK_RESULT(m_Device->CreateBuffer(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &m_UniformBuffers.m_FragLights,
            sizeof(m_LightsUBO)));

        // Map persistent
        CHECK_VK_RESULT(m_UniformBuffers.m_VertFullScreen.Map());

        CHECK_VK_RESULT(m_UniformBuffers.m_FragLights.Map());

        // Update
        UpdateUniformBuffersScreen();
        UpdateLightsUniformBuffer();
    }

    void VulkanRenderer::CreateDescriptorSetLayout()
    {
        // Binding 0 : Vertex shader uniform buffer
        VkDescriptorSetLayoutBinding vertUniformBufferBinding{};
        vertUniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vertUniformBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        vertUniformBufferBinding.binding = 0;
        vertUniformBufferBinding.descriptorCount = 1;

        // Binding 1 : Position
        VkDescriptorSetLayoutBinding positionBinding{};
        positionBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        positionBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        positionBinding.binding = 1;
        positionBinding.descriptorCount = 1;

        // Binding 2 : Normal
        VkDescriptorSetLayoutBinding normalBinding{};
        normalBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        normalBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        normalBinding.binding = 2;
        normalBinding.descriptorCount = 1;

        // Binding 3 : Albedo
        VkDescriptorSetLayoutBinding albedoBinding{};
        albedoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        albedoBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        albedoBinding.binding = 3;
        albedoBinding.descriptorCount = 1;

        VkDescriptorSetLayoutBinding descriptorSetBinding{};
        descriptorSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorSetBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptorSetBinding.binding = 4;
        descriptorSetBinding.descriptorCount = 1;

        // Deferred shading layout
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
        {
            vertUniformBufferBinding,
            positionBinding,
            normalBinding,
            albedoBinding,
            descriptorSetBinding,
        };

        VkDescriptorSetLayoutCreateInfo descriptorLayout{};
        descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayout.pBindings = setLayoutBindings.data();
        descriptorLayout.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

        CHECK_VK_RESULT(vkCreateDescriptorSetLayout(m_Device->GetVulkanDevice(), &descriptorLayout, nullptr, &m_DescriptorSetLayout));

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;

        CHECK_VK_RESULT(vkCreatePipelineLayout(m_Device->GetVulkanDevice(), &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayouts.m_Deferred));

        // Offscreen (scene) rendering pipeline layout
        CHECK_VK_RESULT(vkCreatePipelineLayout(m_Device->GetVulkanDevice(), &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayouts.m_Offscreen));
    }

    void VulkanRenderer::CreatePipelines()
    {
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.flags = 0;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.flags = 0;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = 0xf;
		blendAttachmentState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.front = depthStencilState.back;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.flags = 0;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.flags = 0;

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		dynamicState.flags = 0;

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

        // Output pipeline
        shaderStages[0] = LoadShader("shaders/deferred.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shaderStages[1] = LoadShader("shaders/deferred.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.basePipelineIndex = -1;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.renderPass = m_OutputFrameBuffer->GetRenderPass();
        pipelineCreateInfo.layout = m_PipelineLayouts.m_Deferred;

		//dummy vertex input state to keep validation happy. 
		VkPipelineVertexInputStateCreateInfo inputState = {};
		inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		inputState.vertexBindingDescriptionCount = 0;
		inputState.pVertexBindingDescriptions = nullptr;
		inputState.vertexAttributeDescriptionCount = 0;
		inputState.pVertexAttributeDescriptions = nullptr;

		pipelineCreateInfo.pVertexInputState = &inputState;

        // Blend attachment states required for all color attachments
        // This is important, as color write mask will otherwise be 0x0 and you
        // won't see anything rendered to the attachment
        VkPipelineColorBlendAttachmentState blendAttachmentStateOutput{};
        blendAttachmentStateOutput.colorWriteMask = 0xf;
        blendAttachmentStateOutput.blendEnable = VK_FALSE;

        std::array<VkPipelineColorBlendAttachmentState, 1> blendAttachmentStatesOutput = {
            blendAttachmentStateOutput,
        };

        colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStatesOutput.size());
        colorBlendState.pAttachments = blendAttachmentStatesOutput.data();

        CHECK_VK_RESULT(vkCreateGraphicsPipelines(m_Device->GetVulkanDevice(), m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &m_Pipelines.m_Output));
    }

    void VulkanRenderer::CreateDescriptorPool()
    {
        VkDescriptorPoolSize uniformPoolSize{};
        uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformPoolSize.descriptorCount = 100; //TODO

        VkDescriptorPoolSize imageSamplerPoolSize{};
        imageSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageSamplerPoolSize.descriptorCount = 100; //TODO

        std::vector<VkDescriptorPoolSize> poolSizes =
        {
            uniformPoolSize,
            imageSamplerPoolSize
        };

        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = 100; //TODO

        CHECK_VK_RESULT(vkCreateDescriptorPool(m_Device->GetVulkanDevice(), &descriptorPoolInfo, nullptr, &m_DescriptorPool));
    }

    void VulkanRenderer::CreateDescriptorSet()
    {
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;

        // Textured quad descriptor set
	    VkDescriptorSetAllocateInfo allocInfo = GetDescriptorSetAllocateInfo();

        CHECK_VK_RESULT(vkAllocateDescriptorSets(m_Device->GetVulkanDevice(), &allocInfo, &m_OutputDescriptorSet));

        // Image descriptors for the offscreen color attachments
        VkDescriptorImageInfo texDescriptorPosition{};
        texDescriptorPosition.sampler = m_OffscreenFrameBuffer->GetSampler();
        texDescriptorPosition.imageView = m_OffscreenFrameBuffer->GetAttachment("position").m_ImageView;
        texDescriptorPosition.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo texDescriptorNormal{};
        texDescriptorNormal.sampler = m_OffscreenFrameBuffer->GetSampler();
        texDescriptorNormal.imageView = m_OffscreenFrameBuffer->GetAttachment("normal").m_ImageView;
        texDescriptorNormal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo texDescriptorAlbedo{};
        texDescriptorAlbedo.sampler = m_OffscreenFrameBuffer->GetSampler();
        texDescriptorAlbedo.imageView = m_OffscreenFrameBuffer->GetAttachment("colour").m_ImageView;
        texDescriptorAlbedo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo texDescriptorOutput{};
        texDescriptorOutput.sampler = m_OutputFrameBuffer->GetSampler();
        texDescriptorOutput.imageView = m_OutputFrameBuffer->GetAttachment("colour").m_ImageView;
        texDescriptorOutput.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Binding 0 : Vertex shader uniform buffer
        VkWriteDescriptorSet uniformWriteSet{};
        uniformWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uniformWriteSet.dstSet = m_OutputDescriptorSet;
        uniformWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformWriteSet.dstBinding = 0;
        uniformWriteSet.pBufferInfo = &m_UniformBuffers.m_VertFullScreen.m_Descriptor;
        uniformWriteSet.descriptorCount = 1;

        // Binding 1 : Position texture target
        VkWriteDescriptorSet positionWriteSet{};
        positionWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        positionWriteSet.dstSet = m_OutputDescriptorSet;
        positionWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        positionWriteSet.dstBinding = 1;
        positionWriteSet.pImageInfo = &texDescriptorPosition;
        positionWriteSet.descriptorCount = 1;

        // Binding 2 : Normals texture target
        VkWriteDescriptorSet normalWriteSet{};
        normalWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        normalWriteSet.dstSet = m_OutputDescriptorSet;
        normalWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        normalWriteSet.dstBinding = 2;
        normalWriteSet.pImageInfo = &texDescriptorNormal;
        normalWriteSet.descriptorCount = 1;

        // Binding 3 : Albedo texture target
        VkWriteDescriptorSet albedoWriteSet{};
        albedoWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        albedoWriteSet.dstSet = m_OutputDescriptorSet;
        albedoWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        albedoWriteSet.dstBinding = 3;
        albedoWriteSet.pImageInfo = &texDescriptorAlbedo;
        albedoWriteSet.descriptorCount = 1;

        // Binding 4 : Fragment shader uniform buffer
        VkWriteDescriptorSet fragWriteSet{};
        fragWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        fragWriteSet.dstSet = m_OutputDescriptorSet;
        fragWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        fragWriteSet.dstBinding = 4;
        fragWriteSet.pBufferInfo = &m_UniformBuffers.m_FragLights.m_Descriptor;
        fragWriteSet.descriptorCount = 1;

        //// Binding 0 : output Vertex shader uniform buffer
        //VkWriteDescriptorSet uniformWriteSetOutput{};
        //uniformWriteSetOutput.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //uniformWriteSetOutput.dstSet = m_OutputDescriptorSet;
        //uniformWriteSetOutput.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        //uniformWriteSetOutput.dstBinding = 0;
        //uniformWriteSetOutput.pBufferInfo = &m_UniformBuffers.m_VertFullScreen.m_Descriptor;
        //uniformWriteSetOutput.descriptorCount = 1;
        //
        //// Binding 0 : output colour
        //VkWriteDescriptorSet colourWriteSetOutput{};
        //colourWriteSetOutput.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //colourWriteSetOutput.dstSet = m_OutputDescriptorSet;
        //colourWriteSetOutput.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        //colourWriteSetOutput.dstBinding = 1;
        //colourWriteSetOutput.pImageInfo = &texDescriptorOutput;
        //colourWriteSetOutput.descriptorCount = 1;

        writeDescriptorSets = {
            uniformWriteSet,
            positionWriteSet,
            normalWriteSet,
            albedoWriteSet,
            fragWriteSet,
            //uniformWriteSetOutput,
            //colourWriteSetOutput
        };

        vkUpdateDescriptorSets(m_Device->GetVulkanDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);

    }

    void VulkanRenderer::BuildImguiCommandBuffer(int index)
    {
        m_ImGui->NewFrame();
        m_ImGui->UpdateBuffers();

        // Set target frame buffer
        m_SwapChain->GetCommandBuffer(index)->BeginRecording();
        m_SwapChain->GetCommandBuffer(index)->BeginRenderPass();
        m_ImGui->DrawFrame(m_SwapChain->GetCommandBuffer(index)->GetVulkanCommandBuffer());
        m_SwapChain->GetCommandBuffer(index)->EndRenderPass();
        m_SwapChain->GetCommandBuffer(index)->EndRecording();
    }

    void VulkanRenderer::BuildDefferedCommandBuffer()
    {
        if (!m_OffScreenCmdBuffer)
        {
            m_OffScreenCmdBuffer = CommandBuffer::CreateCommandBuffer();
            m_OffScreenCmdBuffer->SetFrameBuffer(m_OffscreenFrameBuffer);
        }

        if(m_OffscreenSemaphore == VK_NULL_HANDLE)
        {
            // Create a semaphore used to synchronize offscreen rendering and usage
            VkSemaphoreCreateInfo semaphoreCreateInfo{};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            CHECK_VK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &m_OffscreenSemaphore));
        }

        m_OffScreenCmdBuffer->BeginRecording();
        m_OffScreenCmdBuffer->BeginRenderPass();
        m_OffScreenCmdBuffer->SetViewport(m_OffscreenFrameBuffer->GetWidth(), m_OffscreenFrameBuffer->GetHeight(), 0.f, 1.f);
        m_OffScreenCmdBuffer->SetScissor(m_OffscreenFrameBuffer->GetWidth(), m_OffscreenFrameBuffer->GetHeight(), 0, 0);

        for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
        {
            if (ModelComponent* comp = obj->GetComponent<ModelComponent>())
            {
				for (base::Mesh* model : comp->GetModels())
				{
                    if (vk::Mesh* vkModel = static_cast<vk::Mesh*>(model))
                    {
                        vkModel->Render();
                    }
				}
            }
        }

        m_OffScreenCmdBuffer->EndRenderPass();
        m_OffScreenCmdBuffer->EndRecording();
    }

    void VulkanRenderer::BuildOutputFrameBuffer()
    {
        if (!m_OutputCmdBuffer)
        {
            m_OutputCmdBuffer = CommandBuffer::CreateCommandBuffer();
            m_OutputCmdBuffer->SetFrameBuffer(m_OutputFrameBuffer);
        }

        // Create a semaphore used to synchronize offscreen rendering and usage
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        CHECK_VK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &m_OutputSemaphore));

        m_OutputCmdBuffer->BeginRecording();
        m_OutputCmdBuffer->BeginRenderPass();
        m_OutputCmdBuffer->SetViewport(m_SwapChain->GetExtents().width, m_SwapChain->GetExtents().height, 0.f, 1.f);
        m_OutputCmdBuffer->SetScissor(m_SwapChain->GetExtents().width, m_SwapChain->GetExtents().height, 0, 0);

        m_OutputCmdBuffer->BindPipeline(m_Pipelines.m_Output);
        m_OutputCmdBuffer->BindDescriptorSet(m_PipelineLayouts.m_Deferred, m_OutputDescriptorSet);
        m_OutputCmdBuffer->BindVertexBuffer(m_ScreenQuad.GetVertexBuffer());
        m_OutputCmdBuffer->BindIndexBuffer(m_ScreenQuad.GetIndexBuffer());
        m_OutputCmdBuffer->RecordDraw(6);
        m_OutputCmdBuffer->EndRenderPass();
        m_OutputCmdBuffer->EndRecording();
    }

    void VulkanRenderer::UpdateUniformBuffersScreen()
    {
        m_VertUBO.m_Projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
        m_VertUBO.m_Model = glm::mat4(1.0f);

        memcpy(m_UniformBuffers.m_VertFullScreen.m_Mapped, &m_VertUBO, sizeof(m_VertUBO));
    }

    void VulkanRenderer::UpdateLightsUniformBuffer()
    {
        //TODO surely there is a way of only copying the lights that have changed.

        //clear the buffer, this should be smarter at some point.
        memset(&m_LightsUBO, 0, sizeof(m_LightsUBO));

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
                            m_LightsUBO.m_PointLights[pointLightIndex].m_Position = glm::vec4(translationComp->GetTranslation(), 0.f);
                            m_LightsUBO.m_PointLights[pointLightIndex].m_Colour = pointLight->GetColour();
                            m_LightsUBO.m_PointLights[pointLightIndex].m_Radius = pointLight->GetRadius();
                            pointLightIndex++;
                        }
                    }
                    else if (light->GetType() == LightType::Directional)
                    {
                        DirectionalLight* directionalLight = static_cast<DirectionalLight*>(light);
                        m_LightsUBO.m_DirectionalLights[dirLightIndex].m_Direction = directionalLight->GetDirection();
						m_LightsUBO.m_DirectionalLights[dirLightIndex].m_Colour = directionalLight->GetColour();
                    }
                }
            }
        }

        // Current view position
        m_LightsUBO.m_ViewPos = glm::vec4(BaseApplication::Get().GetScene()->GetCamera()->GetPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

        memcpy(m_UniformBuffers.m_FragLights.m_Mapped, &m_LightsUBO, sizeof(m_LightsUBO));
    }



    void VulkanRenderer::SetupImGui()
    {
        m_ImGui = new ImGUIImpl();
        m_ImGui->Init((float)s_Width, (float)s_Height);
        m_ImGui->InitResources(m_SwapChain->GetRenderPass(), GetDevice()->GetGraphicsQueue());
    }

    VkPipelineShaderStageCreateInfo VulkanRenderer::LoadShader(std::string fileName, VkShaderStageFlagBits stage)
    {
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = stage;
        shaderStage.module = CreateShaderModule(Helpers::ReadFile(fileName));
        shaderStage.pName = "main"; // todo : make param
		PL_ASSERT(shaderStage.module != VK_NULL_HANDLE);
        m_ShaderModules.push_back(shaderStage.module);
        return shaderStage;
    }

    VkDescriptorSetAllocateInfo VulkanRenderer::GetDescriptorSetAllocateInfo()
    {
	    VkDescriptorSetAllocateInfo allocInfo{};
	    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	    allocInfo.descriptorPool = m_DescriptorPool;
	    allocInfo.pSetLayouts = &m_DescriptorSetLayout;
	    allocInfo.descriptorSetCount = 1;

	    return allocInfo;
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
		for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
		{
            if (ModelComponent* comp = obj->GetComponent<ModelComponent>())
            {
                for (base::Mesh* model : comp->GetModels())
                {
                    model->Setup(this);
                }
            }
		}
		
		InitLightsVBO();
		BuildDefferedCommandBuffer();
	}

	void VulkanRenderer::OnModelRemovedFromScene()
	{
		InitLightsVBO();
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
		return 		
		{
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_RENDERDOC_Capture"
		};
	}

	void VulkanRenderer::RecreateSwapChain()
	{
        vkDeviceWaitIdle(m_Device->GetVulkanDevice());

        m_SwapChain->Recreate();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(m_SwapChain->GetExtents().width), static_cast<float>(m_SwapChain->GetExtents().height));
	}

}