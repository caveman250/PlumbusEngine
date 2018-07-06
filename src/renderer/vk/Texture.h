#pragma once
#include "vulkan/vulkan.h"
#include <string>
#include "renderer/base/Texture.h"

namespace vk
{
	class Texture : public base::Texture
	{
	public:
		void LoadTexture(std::string filename) override;
		void Cleanup() override;

		void UpdateDescriptor();
		void CreateTextureSampler();

		VkImage m_Image = VK_NULL_HANDLE;
		VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkSampler m_TextureSampler = VK_NULL_HANDLE;
		VkDescriptorImageInfo m_Descriptor;

	};

}