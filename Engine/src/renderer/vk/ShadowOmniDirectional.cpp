#include <TranslationComponent.h>
#include "ShadowOmniDirectional.h"
#include "FrameBuffer.h"
#include "VulkanRenderer.h"
#include "CommandBuffer.h"
#include "BaseApplication.h"
#include "Scene.h"
#include "GameObject.h"
#include "ModelComponent.h"
#include "MaterialInstance.h"
#include "ShadowManager.h"
#include "PipelineLayout.h"
#include "ImageHelpers.h"

namespace plumbus::vk
{
    MaterialRef ShadowOmniDirectional::s_ShadowOmniDirectionalMaterial = nullptr;

    ShadowOmniDirectionalRef ShadowOmniDirectional::CreateShadowOmniDirectional(Light* light)
    {
        ShadowOmniDirectionalRef shadow = std::make_shared<ShadowOmniDirectional>(light);
        shadow->Init();

        return shadow;
    }

    ShadowOmniDirectional::~ShadowOmniDirectional()
    {
        s_ShadowOmniDirectionalMaterial.reset();
        m_ShadowOmniDirectionalMaterialInstance.reset();
        m_UniformBuffer.Cleanup();
    }

    void ShadowOmniDirectional::Init()
    {
        //create framebuffer
        std::vector<FrameBuffer::FrameBufferAttachmentInfo> offscreenAttachmentInfo =
        {
            FrameBuffer::FrameBufferAttachmentInfo(VK_FORMAT_R32_SFLOAT, FrameBuffer::FrameBufferAttachmentType::R32, "dist", true),
            FrameBuffer::FrameBufferAttachmentInfo(VulkanRenderer::Get()->GetDepthFormat(), FrameBuffer::FrameBufferAttachmentType::Depth, "depth", false)
        };

        m_FrameBuffer = FrameBuffer::CreateFrameBuffer(1024, 1024, offscreenAttachmentInfo);

        if (m_CommandBuffer == VK_NULL_HANDLE)
        {
            m_CommandBuffer = CommandBuffer::CreateCommandBuffer();
            m_CommandBuffer->SetFrameBuffer(m_FrameBuffer);
        }

        if (m_Semaphore == VK_NULL_HANDLE)
        {
            VkSemaphoreCreateInfo semaphoreCreateInfo{ };
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            CHECK_VK_RESULT(vkCreateSemaphore(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &m_Semaphore));
        }

        if (!s_ShadowOmniDirectionalMaterial)
        {
            s_ShadowOmniDirectionalMaterial = std::make_shared<Material>("shaders/shadow_omni.vert", "shaders/shadow_omni.frag", GetFrameBuffer()->GetRenderPass());
            s_ShadowOmniDirectionalMaterial->SetCullingMode(VK_CULL_MODE_BACK_BIT);
            s_ShadowOmniDirectionalMaterial->Setup();
        }

        m_ShadowOmniDirectionalMaterialInstance = MaterialInstance::CreateMaterialInstance(s_ShadowOmniDirectionalMaterial);

        SetupCubeMap();
    }

    void ShadowOmniDirectional::BuildCommandBuffer(int index)
    {
        //m_CommandBuffer->BeginRecording();
        m_CommandBuffer->BeginRenderPass();
        m_CommandBuffer->SetViewport((float)m_FrameBuffer->GetWidth(), (float)m_FrameBuffer->GetHeight(), 0.f, 1.f);
        m_CommandBuffer->SetScissor(m_FrameBuffer->GetWidth(), m_FrameBuffer->GetHeight(), 0, 0);

        for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
        {
            if (ModelComponent* comp = obj->GetComponent<ModelComponent>())
            {
                PushContants constants;
                glm::vec3 pos = m_Light->GetParent()->GetOwner()->GetComponent<TranslationComponent>()->GetTranslation();
                glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(-pos.x, -pos.y, -pos.z));
                glm::mat4 rotation = glm::mat4(1.f);
                switch (index)
                {
                    case 0: // POSITIVE_X
                        rotation = glm::rotate(rotation, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                        rotation = glm::rotate(rotation, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                        break;
                    case 1:	// NEGATIVE_X
                        rotation = glm::rotate(rotation, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                        rotation = glm::rotate(rotation, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                        break;
                    case 2:	// POSITIVE_Y
                        rotation = glm::rotate(rotation, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                        break;
                    case 3:	// NEGATIVE_Y
                        rotation = glm::rotate(rotation, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                        break;
                    case 4:	// POSITIVE_Z
                        rotation = glm::rotate(rotation, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                        break;
                    case 5:	// NEGATIVE_Z
                        rotation = glm::rotate(rotation, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                        break;
                }

                constants.view = rotation * translation;
                constants.model = comp->GetModelMatrix();

                vkCmdPushConstants(
                        m_CommandBuffer->GetVulkanCommandBuffer(),
                        m_ShadowOmniDirectionalMaterialInstance->GetMaterial()->GetPipelineLayout()->GetVulkanPipelineLayout(),
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        sizeof(PushContants),
                        &constants);


                vkDeviceWaitIdle(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice());

                for (Mesh* model : comp->GetModels())
                {
                    model->Render(m_CommandBuffer, m_ShadowOmniDirectionalMaterialInstance);
                }
            }
        }

        m_CommandBuffer->EndRenderPass();

        ImageHelpers::SetImageLayout(m_CommandBuffer->GetVulkanCommandBuffer(),
                                     m_FrameBuffer->GetAttachment("dist")->m_Image,
                                     VK_IMAGE_ASPECT_COLOR_BIT,
                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);


        VkImageSubresourceRange cubeFaceSubresourceRange = {};
        cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cubeFaceSubresourceRange.baseMipLevel = 0;
        cubeFaceSubresourceRange.levelCount = 1;
        cubeFaceSubresourceRange.baseArrayLayer = index;
        cubeFaceSubresourceRange.layerCount = 1;

        ImageHelpers::SetImageLayout(m_CommandBuffer->GetVulkanCommandBuffer(),
                                     m_CubeMapTexture.m_Image,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                     cubeFaceSubresourceRange);

        // Copy region for transfer from framebuffer to cube face
        VkImageCopy copyRegion = {};

        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.srcOffset = { 0, 0, 0 };

        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.baseArrayLayer = index;
        copyRegion.dstSubresource.mipLevel = 0;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.dstOffset = { 0, 0, 0 };

        copyRegion.extent.width = 1024;
        copyRegion.extent.height = 1024;
        copyRegion.extent.depth = 1;

        // Put image copy into command buffer
        vkCmdCopyImage(
                m_CommandBuffer->GetVulkanCommandBuffer(),
                m_FrameBuffer->GetAttachment("dist")->m_Image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                m_CubeMapTexture.m_Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &copyRegion);

        // Transform framebuffer color attachment back
        ImageHelpers::SetImageLayout(
                m_CommandBuffer->GetVulkanCommandBuffer(),
                m_FrameBuffer->GetAttachment("dist")->m_Image,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // Change image layout of copied face to shader read
        ImageHelpers::SetImageLayout(
                m_CommandBuffer->GetVulkanCommandBuffer(),
                m_CubeMapTexture.m_Image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                cubeFaceSubresourceRange);
        //m_CommandBuffer->EndRecording();
    }

    void ShadowOmniDirectional::Render(VkSemaphore waitSemaphore)
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

    void ShadowOmniDirectional::UpdateUniformBuffer()
    {
        if (!m_UniformBuffer.IsInitialised())
        {
            CHECK_VK_RESULT(VulkanRenderer::Get()->GetDevice()->CreateBuffer(
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    &m_UniformBuffer,
                    sizeof(UniformBufferObject)));

            CHECK_VK_RESULT(m_UniformBuffer.Map());
        }

        m_UniformBufferObject.m_Proj = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.01f, 1024.f);
        m_UniformBufferObject.m_LightPos = glm::vec4(m_Light->GetParent()->GetOwner()->GetComponent<TranslationComponent>()->GetTranslation(), 1.0f);

        memcpy(m_UniformBuffer.m_Mapped, &m_UniformBufferObject, sizeof(m_UniformBufferObject));

        m_ShadowOmniDirectionalMaterialInstance->SetBufferUniform("UBO", &m_UniformBuffer);

    }

    void ShadowOmniDirectional::SetupCubeMap()
    {
        ImageHelpers::CreateImage(1024,
                                  1024,
                                  VK_FORMAT_R32_SFLOAT,
                                  VK_IMAGE_TILING_OPTIMAL,
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                  m_CubeMapTexture.m_Image,
                                  m_CubeMapTexture.m_ImageMemory,
                                  true);

        CommandBufferRef cmdBuffer = CommandBuffer::CreateCommandBuffer();
        cmdBuffer->BeginRecording();
        // Image barrier for optimal image (target)
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.layerCount = 6;
        ImageHelpers::SetImageLayout(
                cmdBuffer->GetVulkanCommandBuffer(),
                m_CubeMapTexture.m_Image,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                subresourceRange);

        cmdBuffer->Flush();

        m_CubeMapTexture.m_ImageView = ImageHelpers::CreateImageView(m_CubeMapTexture.m_Image, VK_FORMAT_R32_SFLOAT, VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT);
        m_CubeMapTexture.CreateTextureSampler();

    }
}
