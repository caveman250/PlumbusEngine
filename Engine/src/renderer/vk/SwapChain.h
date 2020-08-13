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

		const VkSemaphore& GetImageAvailableSemaphore(uint32_t imageIndex) const { return m_ImageAvailableSemaphores[imageIndex]; }
		const VkSemaphore& GetRenderFinishedSemaphore(uint32_t imageIndex) const { return m_RenderFinishedSemaphores[imageIndex]; }
		const VkFence& GetFence(uint32_t imageIndex) const { return m_Fences[imageIndex]; }
		const VkFence& GetImageInFlightFence(uint32_t imageIndex) const { return m_InFlightImages[imageIndex]; }
		void SetImageInFlightFence(uint32_t imageIndex, const VkFence& fence) { m_InFlightImages[imageIndex] = fence; }

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
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_Fences;
		std::vector<VkFence> m_InFlightImages;
	};
};

