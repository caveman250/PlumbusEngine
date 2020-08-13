#pragma once
#include "plumbus.h"
#include "vulkan/vulkan.h"

namespace plumbus::vk
{
	class FrameBuffer
	{
	public:
		struct FrameBufferAttachmentInfo
		{
			FrameBufferAttachmentInfo(VkFormat format, bool isDepth, std::string name)
				: attachmentFormat(format)
				, depthAttachment(isDepth)
				, attachmentName(name)
			{}

			VkFormat attachmentFormat;
			bool depthAttachment;
			std::string attachmentName;
		};

		struct FrameBufferAttachment
		{
			VkImage m_Image;
			VkDeviceMemory m_Memory;
			VkImageView m_ImageView;
			VkFormat m_Format;
		};

		static FrameBufferRef CreateFrameBuffer(uint32_t width, uint32_t height, std::vector<FrameBufferAttachmentInfo> attachments, VkRenderPass renderPass = VK_NULL_HANDLE);
		static FrameBufferRef CreateFrameBuffer(uint32_t width, uint32_t height, VkRenderPass renderPass, std::vector<VkImageView> attachments, std::vector<VkFormat> attachmentFormats);

		FrameBuffer(int32_t width, int32_t height, bool ownsResources);
		~FrameBuffer();

		const int32_t GetWidth() { return m_Width; }
		const int32_t GetHeight() { return m_Height; }

		const size_t GetAttachmentCount() { return m_Attachments.size(); }
		const FrameBufferAttachment& GetAttachment(std::string attachmentName) { return m_Attachments[attachmentName]; }
		const VkSampler& GetSampler() const { return m_ColourSampler; }
		const VkFramebuffer& GetVulkanFrameBuffer() const { return m_FrameBuffer; }
		const VkRenderPass& GetRenderPass() const { return m_RenderPass; }

		void SetSampler(VkSampler sampler) { m_ColourSampler = sampler; }
		void SetVulkanFrameBuffer(VkFramebuffer frameBuffer) { m_FrameBuffer = frameBuffer; }
		void SetRenderPass(VkRenderPass renderPass) { m_RenderPass = renderPass; }
		void SetOwnsRenderPass(bool ownsRenderPass) { m_OwnsRenderPass = ownsRenderPass; }

		void CreateAttachment(VkFormat format, VkImageUsageFlagBits usage, std::string id);
		void AddAttachment(VkImageView imageView, VkFormat imageFormat, std::string id);

	private:
		int32_t m_Width;
		int32_t m_Height;
		VkFramebuffer m_FrameBuffer;
		std::map<std::string, FrameBufferAttachment> m_Attachments;
		VkRenderPass m_RenderPass;
		VkSampler m_ColourSampler = VK_NULL_HANDLE;

		bool m_OwnsResources;
		bool m_OwnsRenderPass;
	};
}