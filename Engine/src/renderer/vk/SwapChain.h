#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
	class SwapChain
	{
	public:
		static SwapChainRef CreateSwapChain();
		
		SwapChain();
		~SwapChain();

		void Init();
		void Cleanup();
		void Recreate();

		VkExtent2D& GetExtents() { return m_SwapChainExtent; }
		VkSwapchainKHR GetVulkanSwapChain() { return m_SwapChain; }
		std::vector<VkImageView>& GetImageViews() { return m_SwapChainImageViews; }
		VkFormat GetImageFormat() { return m_SwapChainImageFormat; }

		const VkSemaphore& GetImageAvailableSemaphore() const { return m_ImageAvailableSemaphore; }
		const VkSemaphore& GetRenderFinishedSemaphore() const { return m_RenderFinishedSemaphore; }

		const CommandBufferRef& GetCommandBuffer(int index) const { return m_SwapChainCommandBuffers[index]; }
		const VkRenderPass& GetRenderPass() const { return m_PresentRenderPass; }

	private:

		VkSurfaceFormatKHR ChooseFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		VkExtent2D ChooseExtents(const VkSurfaceCapabilitiesKHR& capabilities);

		void CreateVulkanSwapChain();
		void CreateImageViews();
		void CreateRenderPass();
		void CreateDepthTexture();
		void CreateFrameBuffers();
		void CreateSemaphores();

		VkSwapchainKHR m_SwapChain;
		std::vector<VkImage> m_SwapChainImages;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
		std::vector<VkImageView> m_SwapChainImageViews;

		std::vector<FrameBufferRef> m_SwapChainFramebuffers;
		std::vector<CommandBufferRef> m_SwapChainCommandBuffers;
		VkSemaphore m_ImageAvailableSemaphore;
		VkSemaphore m_RenderFinishedSemaphore;

		VkRenderPass m_PresentRenderPass;
		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;
	};
};

