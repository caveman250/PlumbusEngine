#include "CommandBuffer.h"
#include "Helpers.h"
#include "VulkanRenderer.h"

namespace plumbus::vk
{
    CommandBufferRef CommandBuffer::CreateCommandBuffer()
    {
        VkCommandBuffer vkCommandBuffer;
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = VulkanRenderer::Get()->GetDevice()->GetCommandPool();
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        CHECK_VK_RESULT(vkAllocateCommandBuffers(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &commandBufferAllocateInfo, &vkCommandBuffer));

        return std::make_shared<CommandBuffer>(vkCommandBuffer);
    }

	CommandBuffer::CommandBuffer(VkCommandBuffer cmdBuffer)
        : m_CommandBuffer(cmdBuffer)
	{
        
	}

	CommandBuffer::~CommandBuffer()
	{
		Cleanup();
	}

	void CommandBuffer::BeginRecording() const
	{
		VkCommandBufferBeginInfo cmdBufInfo{};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        CHECK_VK_RESULT(vkBeginCommandBuffer(m_CommandBuffer, &cmdBufInfo));
	}

	void CommandBuffer::EndRecording() const
	{
        CHECK_VK_RESULT(vkEndCommandBuffer(m_CommandBuffer));
	}

	void CommandBuffer::Cleanup()
	{
		if (m_CommandBuffer != VK_NULL_HANDLE)
		{
			DeviceRef device = VulkanRenderer::Get()->GetDevice();
			vkFreeCommandBuffers(device->GetVulkanDevice(), device->GetCommandPool(), 1, &m_CommandBuffer);
			m_CommandBuffer = VK_NULL_HANDLE;
		}
	}

	void CommandBuffer::SetViewport(const float width, const float height, const float minDepth, const float maxDepth) const
	{
		VkViewport viewport{};
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);
	}

	void CommandBuffer::SetScissor(const uint32_t width, const uint32_t height, const int32_t offsetX, const int32_t offsetY) const
	{
		VkRect2D scissor{};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = offsetX;
		scissor.offset.y = offsetY;
		vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
	}

	void CommandBuffer::BindPipeline(const VkPipeline piepline) const
	{
		vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, piepline);
	}

	void CommandBuffer::BindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet descriptorSet) const
	{
		vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, NULL);
	}

	void CommandBuffer::BindVertexBuffer(const vk::Buffer& buffer) const
	{
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &buffer.m_Buffer, offsets);
	}

	void CommandBuffer::BindIndexBuffer(const vk::Buffer& buffer) const
	{
		vkCmdBindIndexBuffer(m_CommandBuffer, buffer.m_Buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandBuffer::RecordDraw(const uint32_t indexCount) const
	{
		vkCmdDrawIndexed(m_CommandBuffer, indexCount, 1, 0, 0, 0);
	}

	void CommandBuffer::BeginRenderPass() const
	{
		std::vector<VkClearValue> clearValues;
		clearValues.resize(m_FrameBuffer->GetAttachmentCount());

		//TODO: this should really be smarter.
		//Currently assuming all attachments are colour except for the last one.
		for (int i = 0; i < m_FrameBuffer->GetAttachmentCount(); ++i)
		{
			if (i < (m_FrameBuffer->GetAttachmentCount() - 1))
			{
				clearValues[i].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			}
			else
			{
				clearValues[i].depthStencil = { 1.0f, 0 };
			}
		}

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = m_FrameBuffer->GetRenderPass();
		renderPassBeginInfo.framebuffer = m_FrameBuffer->GetVulkanFrameBuffer();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = m_FrameBuffer->GetWidth();
		renderPassBeginInfo.renderArea.extent.height = m_FrameBuffer->GetHeight();
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::EndRenderPass() const
	{
		vkCmdEndRenderPass(m_CommandBuffer);
	}

}