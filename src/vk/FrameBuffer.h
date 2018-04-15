#pragma once
#include "vulkan/vulkan.h"
namespace vk
{
	class FrameBuffer
	{
	public:

		struct FrameBufferAttachment
		{
			VkImage m_Image;
			VkDeviceMemory m_Memory;
			VkImageView m_ImageView;
			VkFormat m_Format;
		};

		int32_t m_Width;
		int32_t m_Height;
		VkFramebuffer m_FrameBuffer;
		FrameBufferAttachment m_Position;
		FrameBufferAttachment m_Normal;
		FrameBufferAttachment m_Albedo;
		FrameBufferAttachment m_Depth;
		VkRenderPass m_RenderPass;
		VkSampler m_ColourSampler;


		// Create a frame buffer attachment
		void CreateAttachment(
			VkFormat format,
			VkImageUsageFlagBits usage,
			FrameBufferAttachment *attachment);

		// Prepare a new framebuffer and attachments for offscreen rendering (G-Buffer)
		void PrepareOffscreenFramebuffer();
	};
}