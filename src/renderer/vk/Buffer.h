#pragma once
#include "plumbus.h"
#include "vulkan/vulkan.h"
#include "Helpers.h"

namespace plumbus::vk
{
	class Buffer
	{
	public:
		VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void Unmap();
		VkResult Bind(VkDeviceSize offset = 0);
		void SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void CopyTo(void* data, VkDeviceSize size);
		VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void Cleanup();

		VkDevice m_Device;
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo m_Descriptor;
		VkDeviceSize m_Size = 0;
		VkDeviceSize m_Alignment = 0;
		void* m_Mapped = nullptr;

		VkBufferUsageFlags m_UsageFlags;
		VkMemoryPropertyFlags m_MemoryPropertyFlags;
	};
}