#include "ShadowDirectional.h"
#include "FrameBuffer.h"
#include "VulkanRenderer.h"
#include "CommandBuffer.h"
#include "BaseApplication.h"
#include "Scene.h"
#include "GameObject.h"
#include "ModelComponent.h"
#include "MaterialInstance.h"
#include "ShadowManager.h"

namespace plumbus::vk
{
    MaterialRef ShadowDirectional::s_ShadowDirectionalMaterial = nullptr;

    ShadowDirectionalRef ShadowDirectional::CreateShadowDirectional(Light* light) 
    {
        ShadowDirectionalRef shadow = std::make_shared<ShadowDirectional>(light);
        shadow->Init();

        if(!s_ShadowDirectionalMaterial)
        {
             s_ShadowDirectionalMaterial = std::make_shared<Material>("shaders/shadow.vert", "shaders/shadow.frag", shadow->GetFrameBuffer()->GetRenderPass());
             s_ShadowDirectionalMaterial->SetCullingMode(VK_CULL_MODE_FRONT_BIT);
             s_ShadowDirectionalMaterial->Setup();
        }

        return shadow;
    }
	
	ShadowDirectional::~ShadowDirectional()
	{
        s_ShadowDirectionalMaterial.reset();
        m_ShadowDirectionalMaterialInstances.clear();
        m_UniformBufferObjects.clear();
        for(auto& [_, buffer] : m_UniformBuffers)
        {
            buffer.Cleanup();
        }
        m_UniformBuffers.clear();
	}

    void ShadowDirectional::Init() 
    {
        //create framebuffer
        std::vector<FrameBuffer::FrameBufferAttachmentInfo> offscreenAttachmentInfo =
        {
            FrameBuffer::FrameBufferAttachmentInfo(VulkanRenderer::Get()->GetDepthFormat(), FrameBuffer::FrameBufferAttachmentType::Depth, "depth")
        };

        SwapChainRef swapChain = VulkanRenderer::Get()->GetSwapChain();
        m_FrameBuffer = FrameBuffer::CreateFrameBuffer(swapChain->GetExtents().width, swapChain->GetExtents().height, offscreenAttachmentInfo);

    }

    void ShadowDirectional::BuildCommandBuffer() 
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
                DirectionalLight* dirLight = static_cast<DirectionalLight *>(m_Light);
                m_UniformBufferObjects[comp].m_Proj = glm::ortho<float>(-25, 25, -25, 25, -50, 50);
                m_UniformBufferObjects[comp].m_View = glm::lookAt(dirLight->GetDirection(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
                m_UniformBufferObjects[comp].m_Model = comp->GetModelMatrix();

            	if(m_UniformBuffers.count(comp) == 0)
            	{
            		CHECK_VK_RESULT(VulkanRenderer::Get()->GetDevice()->CreateBuffer(
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    &m_UniformBuffers[comp],
                    sizeof(ModelComponent::UniformBufferObject)));

            		CHECK_VK_RESULT(m_UniformBuffers[comp].Map());
            	}

            	if (m_ShadowDirectionalMaterialInstances.count(comp) == 0)
            	{
            		m_ShadowDirectionalMaterialInstances[comp] = MaterialInstance::CreateMaterialInstance(s_ShadowDirectionalMaterial);
            	}
            	

            	m_ShadowDirectionalMaterialInstances[comp]->SetBufferUniform("UBO", &m_UniformBuffers[comp]);
                memcpy(m_UniformBuffers[comp].m_Mapped, &m_UniformBufferObjects[comp], sizeof(m_UniformBufferObjects[comp]));

                vkDeviceWaitIdle(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice());

				for (Mesh* model : comp->GetModels())
				{
                    model->Render(m_CommandBuffer, m_ShadowDirectionalMaterialInstances[comp]);
				}
            }
        }

        m_CommandBuffer->EndRenderPass();
        m_CommandBuffer->EndRecording();
    }
    
    void ShadowDirectional::Render(VkSemaphore waitSemaphore) 
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
}
