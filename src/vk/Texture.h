#pragma once
#include "vulkan/vulkan.h"
#include <string>

namespace vk
{
	class Texture
	{
	public:
		VkImage m_Image = VK_NULL_HANDLE;
		VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkSampler m_TextureSampler = VK_NULL_HANDLE;
		VkDescriptorImageInfo m_Descriptor;

		void UpdateDescriptor();
		void CreateTextureSampler();
		void LoadTexture(std::string filename, VkQueue queue, VkFormat format, VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		void Cleanup(VkDevice device);
	};

}