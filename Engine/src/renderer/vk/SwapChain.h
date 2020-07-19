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

		VkExtent2D& GetExtents() { return m_Extents; }
		VkSwapchainKHR GetVulkanSwapChain() { return m_SwapChain; }

		const VkSemaphore& GetImageAvailableSemaphore() const { return m_ImageAvailableSemaphore; }
		const VkSemaphore& GetRenderFinishedSemaphore() const { return m_RenderFinishedSemaphore; }

		const CommandBufferRef& GetCommandBuffer(int index) const { return m_CommandBuffers[index]; }
		const VkRenderPass& GetRenderPass() const { return m_RenderPass; }

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

		VkFormat m_ImageFormat;
		VkExtent2D m_Extents;
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;

		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;

		std::vector<FrameBufferRef> m_Framebuffers;
		std::vector<CommandBufferRef> m_CommandBuffers;

		VkRenderPass m_RenderPass;
		VkSemaphore m_ImageAvailableSemaphore;
		VkSemaphore m_RenderFinishedSemaphore;
	};
};

