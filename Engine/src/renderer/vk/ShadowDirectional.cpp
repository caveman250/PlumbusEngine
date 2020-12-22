#include "ShadowDirectional.h"
#include "FrameBuffer.h"
#include "VulkanRenderer.h"
#include "CommandBuffer.h"
#include "BaseApplication.h"
#include "Scene.h"
#include "GameObject.h"
#include "ModelComponent.h"
#include "MaterialInstance.h"

namespace plumbus::vk
{
    MaterialRef ShadowDirectional::s_ShadowDirectionalMaterial = nullptr;

    ShadowDirectionalRef ShadowDirectional::CreateShadowDirectional(Light* light) 
    {
        ShadowDirectionalRef shadow = std::make_shared<ShadowDirectional>(light);
        shadow->Init();

        if(!s_ShadowDirectionalMaterial)
        {
             s_ShadowDirectionalMaterial = std::make_shared<Material>("shaders/shadow.vert.spv", "shaders/shadow.frag.spv", shadow->GetFrameBuffer()->GetRenderPass());
             s_ShadowDirectionalMaterial->Setup(Mesh::s_VertexLayout);
        }

        shadow->CreateMaterialInstance();

        return shadow;
    }

    void ShadowDirectional::CreateMaterialInstance() 
    {
        m_ShadowDirectionalMaterialInstance = MaterialInstance::CreateMaterialInstance(s_ShadowDirectionalMaterial);

        CHECK_VK_RESULT(VulkanRenderer::Get()->GetDevice()->CreateBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &m_UniformBuffer,
        sizeof(ModelComponent::UniformBufferObject)));

        CHECK_VK_RESULT(m_UniformBuffer.Map());
    }

    void ShadowDirectional::Init() 
    {
        //create framebuffer
        std::vector<FrameBuffer::FrameBufferAttachmentInfo> offscreenAttachmentInfo =
        {
            FrameBuffer::FrameBufferAttachmentInfo(VulkanRenderer::Get()->GetDepthFormat(), true, "depth")
        };

        SwapChainRef swapChain = VulkanRenderer::Get()->GetSwapChain();
        m_FrameBuffer = FrameBuffer::CreateFrameBuffer(swapChain->GetExtents().width, swapChain->GetExtents().height, offscreenAttachmentInfo);

    }

    void ShadowDirectional::BuildCommandBuffer(uint32_t imageIndex) 
    {
        if (m_CommandBuffer == VK_NULL_HANDLE)
        {
            m_CommandBuffer = CommandBuffer::CreateCommandBuffer();
            m_CommandBuffer->SetFrameBuffer(m_FrameBuffer);
        }

        if (m_Semaphore == VK_NULL_HANDLE)
        {
            VkSemaphoreCreateInfo semaphoreCreateInfo{};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            CHECK_VK_RESULT(vkCreateSemaphore(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &m_Semaphore));
       
        }

        m_CommandBuffer->BeginRecording();
        m_CommandBuffer->BeginRenderPass();
        m_CommandBuffer->SetViewport((float)m_FrameBuffer->GetWidth(), (float)m_FrameBuffer->GetHeight(), 0.f, 1.f);
        m_CommandBuffer->SetScissor(m_FrameBuffer->GetWidth(), m_FrameBuffer->GetHeight(), 0, 0);

        for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
        {
            if (ModelComponent* comp = obj->GetComponent<ModelComponent>())
            {
                DirectionalLight *dirLight = static_cast<DirectionalLight *>(m_Light);
                m_UniformBufferObject.m_Proj = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
                m_UniformBufferObject.m_View = glm::lookAt(dirLight->GetDirection(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
                m_UniformBufferObject.m_Model = comp->GetModelMatrix();

                memcpy(m_UniformBuffer.m_Mapped, &m_UniformBufferObject, sizeof(m_UniformBufferObject));

                m_ShadowDirectionalMaterialInstance->SetBufferUniform("UBO", &m_UniformBuffer);

                vkDeviceWaitIdle(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice());

				for (Mesh* model : comp->GetModels())
				{
                    model->Render(m_CommandBuffer, m_ShadowDirectionalMaterialInstance);
				}
            }
        }

        m_CommandBuffer->EndRenderPass();
        m_CommandBuffer->EndRecording();
    }
    
    void ShadowDirectional::Render(uint32_t imageIndex, VkSemaphore waitSemaphore) 
    {
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { waitSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffer->GetVulkanCommandBuffer();

        VkSemaphore signalSemaphores[] = { m_Semaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        CHECK_VK_RESULT(vkQueueSubmit(VulkanRenderer::Get()->GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
    }

    void ShadowDirectional::CleanupMaterial()
    {
        s_ShadowDirectionalMaterial.reset();
    }
}