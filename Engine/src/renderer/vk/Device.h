#pragma once
#include "plumbus.h"
#include "vulkan/vulkan.h"

namespace plumbus::vk
{
	class Buffer;
	class Device
	{
	public:
		struct QueueFamilyIndices
		{
			int m_GraphicsFamily = -1;
			int m_PresentFamily = -1;

			bool Valid()
			{
				return m_GraphicsFamily >= 0 && m_PresentFamily >= 0;
			}
		};

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR m_Capabilities;
			std::vector<VkSurfaceFormatKHR> m_Formats;
			std::vector<VkPresentModeKHR> m_PresentModes;
		};

		static std::shared_ptr<Device> CreateDevice();
		Device();
		~Device();

		QueueFamilyIndices GetQueueFamilyIndices() { return m_Indices; }
		VkDevice GetVulkanDevice() { return m_Device; }
		VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }
		VkCommandPool GetCommandPool() { return m_CommandPool; }
		VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
		VkQueue GetPresentQueue() { return m_PresentQueue; }

		void CreateLogicalDevice(std::vector<const char*> deviceExtensions, const std::vector<const char*> validationLayers, bool enableValidationLayers);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data = nullptr);
		VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, vk::Buffer *buffer, VkDeviceSize size, void *data = nullptr);
		VkCommandBuffer CreateCommandBuffer(bool begin = true);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);
		VkCommandPool CreateCommandPool();
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

	private:

		void PickPhysicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;
		QueueFamilyIndices m_Indices;
		VkSurfaceKHR m_Surface;
		VkCommandPool m_CommandPool;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;
	};
}