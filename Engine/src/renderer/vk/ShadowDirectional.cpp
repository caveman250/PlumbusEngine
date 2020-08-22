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
             s_ShadowDirectionalMaterial = std::make_shared<Material>("shaders/shadow.vert.spv", "shaders/shadow.frag.spv", shadow->GetFrameBuffer(0)->GetRenderPass());
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

        m_FrameBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (i == 0)
            {
                m_FrameBuffers[i] = FrameBuffer::CreateFrameBuffer(swapChain->GetExtents().width, swapChain->GetExtents().height, offscreenAttachmentInfo);
            }
            else
            {
                m_FrameBuffers[i] = FrameBuffer::CreateFrameBuffer(swapChain->GetExtents().width, swapChain->GetExtents().height, offscreenAttachmentInfo, m_FrameBuffers[0]->GetRenderPass());
            }
        }

    }

    void ShadowDirectional::BuildCommandBuffer(uint32_t imageIndex) 
    {
        if (m_CommandBuffers.empty())
        {
            m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

            for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                m_CommandBuffers[i] = CommandBuffer::CreateCommandBuffer();
                m_CommandBuffers[i]->SetFrameBuffer(m_FrameBuffers[i]);
            }
        }

        if (m_Semaphores.empty())
        {
            m_Semaphores.resize(MAX_FRAMES_IN_FLIGHT);

            for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
            {
                VkSemaphoreCreateInfo semaphoreCreateInfo{};
                semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                CHECK_VK_RESULT(vkCreateSemaphore(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &m_Semaphores[i]));
            }
        }

        m_CommandBuffers[imageIndex]->BeginRecording();
        m_CommandBuffers[imageIndex]->BeginRenderPass();
        m_CommandBuffers[imageIndex]->SetViewport((float)m_FrameBuffers[imageIndex]->GetWidth(), (float)m_FrameBuffers[imageIndex]->GetHeight(), 0.f, 1.f);
        m_CommandBuffers[imageIndex]->SetScissor(m_FrameBuffers[imageIndex]->GetWidth(), m_FrameBuffers[imageIndex]->GetHeight(), 0, 0);

        for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
        {
            if (ModelComponent* comp = obj->GetComponent<ModelComponent>())
            {
                if(obj->GetID() == "plane")
                {
                    continue;
                }
                DirectionalLight *dirLight = static_cast<DirectionalLight *>(m_Light);
                m_UniformBufferObject.m_Proj = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
                m_UniformBufferObject.m_View = glm::lookAt(dirLight->GetDirection(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
                m_UniformBufferObject.m_Model = comp->GetModelMatrix();

                memcpy(m_UniformBuffer.m_Mapped, &m_UniformBufferObject, sizeof(m_UniformBufferObject));

                m_ShadowDirectionalMaterialInstance->SetBufferUniform("UBO", &m_UniformBuffer);

				for (Mesh* model : comp->GetModels())
				{
                    model->Render(m_CommandBuffers[imageIndex], m_ShadowDirectionalMaterialInstance);
				}
            }
        }

        m_CommandBuffers[imageIndex]->EndRenderPass();
        m_CommandBuffers[imageIndex]->EndRecording();
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
        submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex]->GetVulkanCommandBuffer();

        VkSemaphore signalSemaphores[] = { m_Semaphores[imageIndex] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        CHECK_VK_RESULT(vkQueueSubmit(VulkanRenderer::Get()->GetDevice()->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
    }

    void ShadowDirectional::CleanupMaterial()
    {
        s_ShadowDirectionalMaterial.reset();
    }
}