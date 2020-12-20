#include "plumbus.h"
#include "FrameBuffer.h"
#include "Helpers.h"
#include "BaseApplication.h"
#include "Device.h"
#include "renderer/vk/VulkanRenderer.h"

namespace plumbus::vk
{
	FrameBuffer::FrameBuffer(int32_t width, int32_t height, bool ownsResources)
		: m_Width(width)
		, m_Height(height)
		, m_OwnsResources(ownsResources)
	{

	}

	FrameBuffer::~FrameBuffer()
	{
		VkDevice device = VulkanRenderer::Get()->GetDevice()->GetVulkanDevice();

		if (m_OwnsResources)
		{
			for (auto& [_, attachment] : m_Attachments)
			{
				if (attachment.m_ImageView != VK_NULL_HANDLE)
				{
					vkDestroyImageView(device, attachment.m_ImageView, nullptr);
					attachment.m_ImageView = VK_NULL_HANDLE;
				}

				if (attachment.m_Image != VK_NULL_HANDLE)
				{
					vkDestroyImage(device, attachment.m_Image, nullptr);
					attachment.m_Image = VK_NULL_HANDLE;
				}

				if (attachment.m_Memory != VK_NULL_HANDLE)
				{
					vkFreeMemory(device, attachment.m_Memory, nullptr);
					attachment.m_Memory = VK_NULL_HANDLE;
				}
			}

			if (m_ColourSampler != VK_NULL_HANDLE)
			{
				vkDestroySampler(device, m_ColourSampler, nullptr);
			}

			vkDestroyRenderPass(device, m_RenderPass, nullptr);
		}

		vkDestroyFramebuffer(device, m_FrameBuffer, nullptr);
	}

	void FrameBuffer::CreateAttachment(VkFormat format, VkImageUsageFlagBits usage, std::string id)
	{
		VkImageAspectFlags aspectMask = 0;
		VkImageLayout imageLayout;

		m_Attachments[id] = FrameBufferAttachment();
		FrameBufferAttachment& attachment = m_Attachments[id];

		attachment.m_Format = format;

		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		PL_ASSERT(aspectMask > 0);

		VkImageCreateInfo image{ };
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = format;
		image.extent.width = m_Width;
		image.extent.height = m_Height;
		image.extent.depth = 1;
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

		std::shared_ptr<vk::Device> device = VulkanRenderer::Get()->GetDevice();

		VkMemoryAllocateInfo memAlloc{};
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs;

		CHECK_VK_RESULT(vkCreateImage(device->GetVulkanDevice(), &image, nullptr, &attachment.m_Image));
		vkGetImageMemoryRequirements(device->GetVulkanDevice(), attachment.m_Image, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = device->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		CHECK_VK_RESULT(vkAllocateMemory(device->GetVulkanDevice(), &memAlloc, nullptr, &attachment.m_Memory));
		CHECK_VK_RESULT(vkBindImageMemory(device->GetVulkanDevice(), attachment.m_Image, attachment.m_Memory, 0));

		VkImageViewCreateInfo imageView{};
		imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageView.format = format;
		imageView.subresourceRange = {};
		imageView.subresourceRange.aspectMask = aspectMask;
		imageView.subresourceRange.baseMipLevel = 0;
		imageView.subresourceRange.levelCount = 1;
		imageView.subresourceRange.baseArrayLayer = 0;
		imageView.subresourceRange.layerCount = 1;
		imageView.image = attachment.m_Image;
		CHECK_VK_RESULT(vkCreateImageView(device->GetVulkanDevice(), &imageView, nullptr, &attachment.m_ImageView));
	}

	void FrameBuffer::AddAttachment(VkImageView imageView, VkFormat imageFormat, std::string id)
	{
		m_Attachments[id] = FrameBufferAttachment();
		FrameBufferAttachment& attachment = m_Attachments[id];

		attachment.m_Format = imageFormat;
		attachment.m_ImageView = imageView;
	}

	plumbus::vk::FrameBufferRef FrameBuffer::CreateFrameBuffer(uint32_t width, uint32_t height, VkRenderPass renderPass, std::vector<VkImageView> attachments, std::vector<VkFormat> attachmentFormats)
	{
		PL_ASSERT(attachments.size() == attachmentFormats.size());

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = NULL;
		frameBufferCreateInfo.renderPass = renderPass;
		frameBufferCreateInfo.attachmentCount = (uint32_t)attachments.size();
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width = width;
		frameBufferCreateInfo.height = height;
		frameBufferCreateInfo.layers = 1;
		VkFramebuffer frameBufferObj;
		vkCreateFramebuffer(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &frameBufferCreateInfo, nullptr, &frameBufferObj);

		FrameBufferRef fb = std::make_shared<FrameBuffer>(width, height, false);
		fb->SetRenderPass(renderPass);
		fb->SetVulkanFrameBuffer(frameBufferObj);
		for (int i = 0; i < attachments.size(); ++i)
		{
			fb->AddAttachment(attachments[i], attachmentFormats[i], std::to_string(i));
		}

		return fb;
	}

	plumbus::vk::FrameBufferRef FrameBuffer::CreateFrameBuffer(uint32_t width, uint32_t height, std::vector<FrameBufferAttachmentInfo> attachments)
	{
		vk::VulkanRenderer* renderer = VulkanRenderer::Get();
		std::shared_ptr<vk::Device> device = renderer->GetDevice();
		
		FrameBufferRef fb = std::make_shared<FrameBuffer>(width, height, true);

		// Set up separate renderpass with references to the color and depth attachments
		std::vector<VkAttachmentDescription> attachmentDescs = {};
		attachmentDescs.resize(attachments.size());

		std::vector<VkImageView> attachmentImageViews = {};
		attachmentImageViews.resize(attachments.size());

		std::vector<VkAttachmentReference> colorReferences;

		int depthIndex = 0;
		for (int i = 0; i < attachments.size(); ++i)
		{
			fb->CreateAttachment(attachments[i].attachmentFormat, !attachments[i].depthAttachment ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, attachments[i].attachmentName);

			attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			if (attachments[i].depthAttachment)
			{
				depthIndex = i;
				attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			}
			else
			{
				attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				colorReferences.push_back({ (uint32_t)i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			}

			attachmentDescs[i].format = attachments[i].attachmentFormat;

			attachmentImageViews[i] = fb->GetAttachment(attachments[i].attachmentName).m_ImageView;
		}

		//CREATE RENDER PASS
		VkAttachmentReference depthReference = {};
		depthReference.attachment = depthIndex;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = colorReferences.data();
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
		subpass.pDepthStencilAttachment = &depthReference;

		// Use subpass dependencies for attachment layput transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachmentDescs.data();
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();

		VkRenderPass renderPass;
		CHECK_VK_RESULT(vkCreateRenderPass(device->GetVulkanDevice(), &renderPassInfo, nullptr, &renderPass));
		fb->SetRenderPass(renderPass);


		//CREATE FRAMEBUFFER
		VkFramebufferCreateInfo fbCreateInfo = {};
		fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbCreateInfo.pNext = NULL;
		fbCreateInfo.renderPass = renderPass;
		fbCreateInfo.pAttachments = attachmentImageViews.data();
		fbCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentImageViews.size());
		fbCreateInfo.width = width;
		fbCreateInfo.height = height;
		fbCreateInfo.layers = 1;

		VkFramebuffer frameBuffer;
		CHECK_VK_RESULT(vkCreateFramebuffer(device->GetVulkanDevice(), &fbCreateInfo, nullptr, &frameBuffer));
		fb->SetVulkanFrameBuffer(frameBuffer);

		//CREATE SAMPLER
		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 1.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VkSampler sampler;
		CHECK_VK_RESULT(vkCreateSampler(device->GetVulkanDevice(), &samplerCreateInfo, nullptr, &sampler));
		fb->SetSampler(sampler);

		return fb;
	}

}