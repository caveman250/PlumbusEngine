#pragma once
#include "plumbus.h"
#include "vulkan/vulkan.h"

namespace plumbus::vk
{
	class Buffer;
	class VulkanDevice
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

		VulkanDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		~VulkanDevice();

		QueueFamilyIndices GetQueueFamilyIndices() { return m_Indices; }
		VkDevice GetDevice() { return m_Device; }
		VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }
		VkCommandPool GetCommandPool() { return m_CommandPool; }

		void CreateLogicalDevice(std::vector<const char*> deviceExtensions, const std::vector<const char*> validationLayers, bool enableValidationLayers);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data = nullptr);
		VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, vk::Buffer *buffer, VkDeviceSize size, void *data = nullptr);
		VkCommandBuffer CreateCommandBuffer(bool begin = true);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);
		VkCommandPool CreateCommandPool();

	private:
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;
		QueueFamilyIndices m_Indices;
		VkSurfaceKHR m_Surface;
		VkCommandPool m_CommandPool;
	};
}