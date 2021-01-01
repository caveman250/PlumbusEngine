#pragma once
#include "plumbus.h"
#include "vulkan/vulkan.h"

namespace plumbus::vk
{
    enum class TextureType
    {
        Depth32U,
        Depth32,
        Depth24,
        R32,
        R32Cube,
        RGB8,
        RGB16,
        RGB32,
        RGBA8,
        RGBA16,
        RGBA32
    };

    static bool IsDepthFormat(const TextureType& type)
    {
        return type == TextureType::Depth24 ||
                type == TextureType::Depth32 ||
                type == TextureType::Depth32U;
    }

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