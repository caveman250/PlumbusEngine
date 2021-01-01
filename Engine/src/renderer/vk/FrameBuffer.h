#pragma once
#include "plumbus.h"
#include "vulkan/vulkan.h"

namespace plumbus::vk
{
	class FrameBuffer
	{
	public:
	    enum class FrameBufferAttachmentType
        {
	        Colour,
	        Depth,
	        ColourCube,
	        DepthCube,
	        R32,
	        R32Cube
        };

		struct FrameBufferAttachmentInfo
		{
			FrameBufferAttachmentInfo(VkFormat format, FrameBufferAttachmentType type, std::string name, bool transferSrc = false)
				: attachmentFormat(format)
				, attachmentType(type)
				, attachmentName(std::move(name))
				, transferSrc(transferSrc)
			{}

			VkImageUsageFlagBits GetUsageFlagBits();
			VkImageLayout  GetImageLayout();
            VkImageViewType GetImageViewType();
            int GetLayerCount();

			VkFormat attachmentFormat;
			FrameBufferAttachmentType attachmentType;
			std::string attachmentName;
			bool transferSrc;
		};

		struct FrameBufferAttachment
		{
			VkImage m_Image;
			VkDeviceMemory m_Memory;
			VkImageView m_ImageView;
			VkFormat m_Format;

			bool IsDepth() const
            {
			    return m_Format == VK_FORMAT_D16_UNORM ||
			    m_Format == VK_FORMAT_D16_UNORM ||
                m_Format == VK_FORMAT_D16_UNORM_S8_UINT ||
			    m_Format == VK_FORMAT_D24_UNORM_S8_UINT ||
			    m_Format == VK_FORMAT_D32_SFLOAT ||
			    m_Format == VK_FORMAT_D32_SFLOAT_S8_UINT;
            }
		};

		static FrameBufferRef CreateFrameBuffer(uint32_t width, uint32_t height, std::vector<FrameBufferAttachmentInfo> attachments);
		static FrameBufferRef CreateFrameBuffer(uint32_t width, uint32_t height, VkRenderPass renderPass, std::vector<VkImageView> attachments, std::vector<VkFormat> attachmentFormats);

		FrameBuffer(int32_t width, int32_t height, bool ownsResources);
		~FrameBuffer();

		const int32_t GetWidth() { return m_Width; }
		const int32_t GetHeight() { return m_Height; }

		const size_t GetAttachmentCount() { return m_Attachments.size(); }
		const FrameBufferAttachment* GetAttachment(const std::string& attachmentName);
        const std::vector<std::pair<std::string, FrameBufferAttachment>>& GetAttachments() { return m_Attachments; }
		const VkSampler& GetSampler() const { return m_ColourSampler; }
		const VkFramebuffer& GetVulkanFrameBuffer() const { return m_FrameBuffer; }
		const VkRenderPass& GetRenderPass() const { return m_RenderPass; }

		void SetSampler(VkSampler sampler) { m_ColourSampler = sampler; }
		void SetVulkanFrameBuffer(VkFramebuffer frameBuffer) { m_FrameBuffer = frameBuffer; }
		void SetRenderPass(VkRenderPass renderPass) { m_RenderPass = renderPass; }

		void CreateAttachment(VkImageViewType imageType, VkFormat format, VkImageUsageFlagBits usage, int layerCount, std::string id);
		void AddAttachment(VkImageView imageView, VkFormat imageFormat, std::string id);

	private:
		int32_t m_Width;
		int32_t m_Height;
		VkFramebuffer m_FrameBuffer;
		std::vector<std::pair<std::string, FrameBufferAttachment>> m_Attachments;
		VkRenderPass m_RenderPass;
		VkSampler m_ColourSampler = VK_NULL_HANDLE;

		bool m_OwnsResources;
	};
}