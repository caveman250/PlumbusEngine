#pragma once
#include "vulkan/vulkan.h"
#include <map>
#include <string>
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
		std::map<std::string, FrameBufferAttachment> m_Attachments;
		VkRenderPass m_RenderPass;
		VkSampler m_ColourSampler;


		// Create a frame buffer attachment
		void CreateAttachment(VkFormat format, VkImageUsageFlagBits usage, const char* id);

		// Prepare a new framebuffer and attachments for offscreen rendering (G-Buffer)
		void PrepareOffscreenFramebuffer();
		//used as a render target instead of the swap chain when using IMGui
		void PrepareOutputFramebuffer();
	};
}