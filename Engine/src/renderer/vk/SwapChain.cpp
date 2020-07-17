#include "SwapChain.h"
#include "Device.h"
#include "VulkanRenderer.h"
#include "ImageHelpers.h"

namespace plumbus::vk
{
	std::shared_ptr<SwapChain> SwapChain::CreateSwapChain()
	{
		return std::make_shared<SwapChain>();
	}

	SwapChain::SwapChain()
	{
		CreateVulkanSwapChain();
		CreateImageViews();
	}

	SwapChain::~SwapChain()
	{

	}

	void SwapChain::CreateVulkanSwapChain()
	{
		VulkanRenderer* renderer = VulkanRenderer::Get();
		std::shared_ptr<Device> device = renderer->GetDevice();

		Device::SwapChainSupportDetails swapChainSupport = device->QuerySwapChainSupport(device->GetPhysicalDevice());

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.m_Formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.m_PresentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.m_Capabilities);

		uint32_t imageCount = swapChainSupport.m_Capabilities.minImageCount + 1;
		if (swapChainSupport.m_Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.m_Capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.m_Capabilities.maxImageCount;
			Log::Info("Max image count: ", imageCount);
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = renderer->GetVulkanWindow()->GetSurface();
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
		m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device->GetVulkanDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());
		m_SwapChainImageFormat = surfaceFormat.format;
		m_SwapChainExtent = extent;
	}

	void SwapChain::RecreateSwapChain()
	{
		Window* window = VulkanRenderer::Get()->GetVulkanWindow();
		std::shared_ptr<Device> device = VulkanRenderer::Get()->GetDevice();

		if (window->GetWidth() == 0 || window->GetHeight() == 0)
			return;

		Log::Info("Recreating Swapchain: %i x %i", window->GetWidth(), window->GetHeight());

		vkDeviceWaitIdle(device->GetVulkanDevice());

		CleanupSwapChain();

		vkDeviceWaitIdle(device->GetVulkanDevice());

		CreateVulkanSwapChain();

		CreateImageViews();
	}

	void SwapChain::CleanupSwapChain()
	{
		std::shared_ptr<Device> device = VulkanRenderer::Get()->GetDevice();

		for (size_t i = 0; i < m_SwapChainImageViews.size(); i++)
		{
			vkDestroyImageView(device->GetVulkanDevice(), m_SwapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(device->GetVulkanDevice(), m_SwapChain, nullptr);
	}

	void SwapChain::CreateImageViews()
	{
		m_SwapChainImageViews.resize(m_SwapChainImages.size());
		for (size_t i = 0; i < m_SwapChainImages.size(); i++)
		{
			m_SwapChainImageViews[i] = ImageHelpers::CreateImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

	VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
	{
		return VK_PRESENT_MODE_IMMEDIATE_KHR;

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

	VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		Window* window = VulkanRenderer::Get()->GetVulkanWindow();

		VkExtent2D actualExtent = { window->GetWidth(), window->GetHeight() };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

}