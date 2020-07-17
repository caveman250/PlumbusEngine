#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
	class SwapChain
	{
	public:
		static std::shared_ptr<SwapChain> CreateSwapChain();
		
		SwapChain();
		~SwapChain();

		void CreateVulkanSwapChain();
		void RecreateSwapChain();
		void CleanupSwapChain();
		void CreateImageViews();

		VkExtent2D& GetSwapChainExtent() { return m_SwapChainExtent; }
		VkSwapchainKHR GetVulkanSwapChain() { return m_SwapChain; }
		std::vector<VkImageView>& GetImageViews() { return m_SwapChainImageViews; }
		VkFormat GetSwapChainImageFormat() { return m_SwapChainImageFormat; }
	private:

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkSwapchainKHR m_SwapChain;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
		std::vector<VkImageView> m_SwapChainImageViews;
	};
};

