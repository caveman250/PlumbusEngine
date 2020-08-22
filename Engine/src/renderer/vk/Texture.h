#pragma once
#include "plumbus.h"
#include "vulkan/vulkan.h"

namespace plumbus::vk
{
	class Texture
	{
	public:
		void LoadTexture(std::string filename);
		void Cleanup();

		void CreateTextureSampler();

		VkImage m_Image = VK_NULL_HANDLE;
		VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkSampler m_TextureSampler = VK_NULL_HANDLE;

	};

}