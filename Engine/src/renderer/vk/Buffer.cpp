#include "plumbus.h"
#include "Buffer.h"

namespace plumbus::vk
{
	VkResult Buffer::Map(VkDeviceSize size , VkDeviceSize offset)
	{
		return vkMapMemory(m_Device, m_Memory, offset, size, 0, &m_Mapped);
	}

	void Buffer::Unmap()
	{
		if (m_Mapped)
		{
			vkUnmapMemory(m_Device, m_Memory);
			m_Mapped = nullptr;
		}
	}

	VkResult Buffer::Bind(VkDeviceSize offset /*= 0*/)
	{
		return vkBindBufferMemory(m_Device, m_Buffer, m_Memory, offset);
	}

	void Buffer::SetupDescriptor(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/)
	{
		m_Descriptor.offset = offset;
		m_Descriptor.buffer = m_Buffer;
		m_Descriptor.range = size;
	}

	void Buffer::CopyTo(void* data, VkDeviceSize size)
	{
		PL_ASSERT(m_Mapped);
		memcpy(m_Mapped, data, size);
	}

	VkResult Buffer::Flush(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = m_Memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(m_Device, 1, &mappedRange);
	}

	VkResult Buffer::Invalidate(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = m_Memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(m_Device, 1, &mappedRange);
	}

	void Buffer::Cleanup()
	{
		if (m_Buffer)
		{
			vkDestroyBuffer(m_Device, m_Buffer, nullptr);
		}
		if (m_Memory)
		{
			vkFreeMemory(m_Device, m_Memory, nullptr);
		}
	}

}