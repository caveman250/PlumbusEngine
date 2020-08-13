#include "SwapChain.h"
#include "Device.h"
#include "VulkanRenderer.h"
#include "ImageHelpers.h"
#include "CommandBuffer.h"

namespace plumbus::vk
{
	SwapChainRef SwapChain::CreateSwapChain()
	{
		return std::make_shared<SwapChain>();
	}

	SwapChain::SwapChain()
	{
		Init();
	}

	SwapChain::~SwapChain()
	{

	}

	void SwapChain::Init()
	{
		CreateVulkanSwapChain();
		CreateImageViews();

		for (int i = 0; i < m_ImageViews.size(); ++i)
		{
			m_CommandBuffers.push_back(CommandBuffer::CreateCommandBuffer());
		}

		CreateRenderPass();
		CreateDepthTexture();
		CreateFrameBuffers();
		CreateSemaphores();
	}

	void SwapChain::Cleanup()
	{
		DeviceRef device = VulkanRenderer::Get()->GetDevice();

		for (size_t i = 0; i < m_ImageViews.size(); i++)
		{
			vkDestroyImageView(device->GetVulkanDevice(), m_ImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(device->GetVulkanDevice(), m_SwapChain, nullptr);

		vkDestroyImageView(device->GetVulkanDevice(), m_DepthImageView, nullptr);
		vkDestroyImage(device->GetVulkanDevice(), m_DepthImage, nullptr);
		vkFreeMemory(device->GetVulkanDevice(), m_DepthImageMemory, nullptr);

		m_Framebuffers.clear();
		m_CommandBuffers.clear();

		vkDestroyRenderPass(device->GetVulkanDevice(), m_RenderPass, nullptr);

		for(int i = 0; i < m_Images.size(); ++i)
		{
			vkDestroySemaphore(device->GetVulkanDevice(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device->GetVulkanDevice(), m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device->GetVulkanDevice(), m_Fences[i], nullptr); 
			m_InFlightImages[i] = VK_NULL_HANDLE;
		}
	}

	void SwapChain::Recreate()
	{
		Window* window = VulkanRenderer::Get()->GetWindow();
		DeviceRef device = VulkanRenderer::Get()->GetDevice();

		if (window->GetWidth() == 0 || window->GetHeight() == 0)
			return;

		Log::Info("Recreating Swapchain: %i x %i", window->GetWidth(), window->GetHeight());

		Cleanup();

		vkDeviceWaitIdle(device->GetVulkanDevice());

		Init();
	}

	void SwapChain::CreateVulkanSwapChain()
	{
		VulkanRenderer* renderer = VulkanRenderer::Get();
		DeviceRef device = renderer->GetDevice();

		Device::SwapChainSupportDetails swapChainSupport = device->QuerySwapChainSupport(device->GetPhysicalDevice());

		VkSurfaceFormatKHR surfaceFormat = ChooseFormat(swapChainSupport.m_Formats);
		VkPresentModeKHR presentMode = ChoosePresentMode(swapChainSupport.m_PresentModes);
		VkExtent2D extent = ChooseExtents(swapChainSupport.m_Capabilities);

		PL_ASSERT(MAX_FRAMES_IN_FLIGHT >= swapChainSupport.m_Capabilities.minImageCount);
		PL_ASSERT(MAX_FRAMES_IN_FLIGHT <= swapChainSupport.m_Capabilities.maxImageCount);
		uint32_t imageCount = glm::clamp(swapChainSupport.m_Capabilities.maxImageCount, swapChainSupport.m_Capabilities.minImageCount, (unsigned int)MAX_FRAMES_IN_FLIGHT);

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = renderer->GetWindow()->GetSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		vk::Device::QueueFamilyIndices indices = device->GetQueueFamilyIndices();
		uint32_t queueFamilyIndices[] = { (uint32_t)indices.m_GraphicsFamily, (uint32_t)indices.m_PresentFamily };

		if (indices.m_GraphicsFamily != indices.m_PresentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.m_Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device->GetVulkanDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
		{
			Log::Fatal("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device->GetVulkanDevice(), m_SwapChain, &imageCount, nullptr);
		m_Images.resize(imageCount);
		vkGetSwapchainImagesKHR(device->GetVulkanDevice(), m_SwapChain, &imageCount, m_Images.data());
		m_ImageFormat = surfaceFormat.format;
		m_Extents = extent;
	}

	void SwapChain::CreateImageViews()
	{
		m_ImageViews.resize(m_Images.size());
		for (size_t i = 0; i < m_Images.size(); i++)
		{
			m_ImageViews[i] = ImageHelpers::CreateImageView(m_Images[i], m_ImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	VkSurfaceFormatKHR SwapChain::ChooseFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR SwapChain::ChoosePresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
	{
		//standard double buffering.
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				//double buffering but the queue doesnt get blocked if we fill it up to fast unlike VK_PRESENT_MODE_FIFO_KHR
				return availablePresentMode;
			}
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				//everything gets sent to screen straight away, not ideal but most widely supported, 
				//this should be settings driven at some point.
				bestMode = availablePresentMode;
			}
		}

		return bestMode;
	}

	VkExtent2D SwapChain::ChooseExtents(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		Window* window = VulkanRenderer::Get()->GetWindow();

		VkExtent2D actualExtent = { window->GetWidth(), window->GetHeight() };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

	void SwapChain::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = m_ImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = VulkanRenderer::Get()->GetDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
		{
			Log::Fatal("failed to create render pass!");
		}
	}

	void SwapChain::CreateDepthTexture()
	{
		VkFormat depthFormat = VulkanRenderer::Get()->GetDepthFormat();
		ImageHelpers::CreateImage(GetExtents().width,
			GetExtents().height,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_DepthImage,
			m_DepthImageMemory);

		m_DepthImageView = ImageHelpers::CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		ImageHelpers::TransitionImageLayout(m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VulkanRenderer::Get()->GetDevice()->GetGraphicsQueue());
	}

	void SwapChain::CreateFrameBuffers()
	{
		m_Framebuffers.resize(m_ImageViews.size());
		for (int i = 0; i < m_Framebuffers.size(); ++i)
		{
			std::vector<VkImageView> attachments = { m_ImageViews[i], m_DepthImageView };
			std::vector<VkFormat> attachmentFormats = { m_ImageFormat, VulkanRenderer::Get()->GetDepthFormat() };
			m_Framebuffers[i] = FrameBuffer::CreateFrameBuffer(GetExtents().width, GetExtents().height, m_RenderPass, attachments, attachmentFormats);
			m_CommandBuffers[i]->SetFrameBuffer(m_Framebuffers[i]);
		}
	}

	void SwapChain::CreateSemaphores()
	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
    	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkDevice device = VulkanRenderer::Get()->GetDevice()->GetVulkanDevice();

		m_ImageAvailableSemaphores.resize(m_Images.size());
		m_RenderFinishedSemaphores.resize(m_Images.size());
		m_Fences.resize(m_Images.size());
		m_InFlightImages.resize(m_Images.size(), VK_NULL_HANDLE);
		for(int i = 0; i < m_Images.size(); ++i)
		{
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &m_Fences[i]) != VK_SUCCESS)
			{
				Log::Fatal("failed to create semaphores!");
			}
		}
	}

}