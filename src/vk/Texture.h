#pragma once
#include "vulkan/vulkan.h"
#include <string>

namespace vk
{
	class Texture
	{
	public:
		VkImage m_Image;
		VkDeviceMemory m_ImageMemory;
		VkImageView m_ImageView;
		VkSampler m_TextureSampler;
		VkDescriptorImageInfo m_Descriptor;

		void UpdateDescriptor();
		void CreateTextureSampler();
		void LoadTexture(std::string filename, VkQueue queue, VkFormat format, VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		void Cleanup(VkDevice device);
	};

}