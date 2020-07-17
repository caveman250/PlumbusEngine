#include "plumbus.h"

#include "renderer/vk/Texture.h"
#include "BaseApplication.h"
#include "gli/gli.hpp"
#include "renderer/vk/ImageHelpers.h"

namespace plumbus::vk
{
	void Texture::UpdateDescriptor()
	{
		m_Descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_Descriptor.imageView = m_ImageView;
		m_Descriptor.sampler = m_TextureSampler;
	}

	void Texture::CreateTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
		{
			Log::Error("failed to create texture sampler!");
		}
	}

	void Texture::LoadTexture(std::string filename)
	{
		VkFormat format = VK_FORMAT_BC3_UNORM_BLOCK;
		VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkQueue queue = VulkanRenderer::Get()->GetDevice()->GetGraphicsQueue();

		gli::texture2d tex2D(gli::load(filename.c_str()));

		PLUMBUS_ASSERT(!tex2D.empty());

		std::shared_ptr<vk::Device> device = VulkanRenderer::Get()->GetDevice();

		uint32_t width = static_cast<uint32_t>(tex2D[0].extent().x);
		uint32_t height = static_cast<uint32_t>(tex2D[0].extent().y);
		uint32_t mipLevels = static_cast<uint32_t>(tex2D.levels());

		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs;

		VkCommandBuffer copyCmd = device->CreateCommandBuffer();

		// Create a staging buffer for the image data
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = tex2D.size();
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		CHECK_VK_RESULT(vkCreateBuffer(device->GetVulkanDevice(), &bufferCreateInfo, nullptr, &stagingBuffer));
		vkGetBufferMemoryRequirements(device->GetVulkanDevice(), stagingBuffer, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = device->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		CHECK_VK_RESULT(vkAllocateMemory(device->GetVulkanDevice(), &memAllocInfo, nullptr, &stagingMemory));
		CHECK_VK_RESULT(vkBindBufferMemory(device->GetVulkanDevice(), stagingBuffer, stagingMemory, 0));

		// Copy texture data into staging buffer
		uint8_t *data;
		CHECK_VK_RESULT(vkMapMemory(device->GetVulkanDevice(), stagingMemory, 0, memReqs.size, 0, (void **)&data));
		memcpy(data, tex2D.data(), tex2D.size());
		vkUnmapMemory(device->GetVulkanDevice(), stagingMemory);

		// Setup copy regions for mip levels
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;

		for (uint32_t i = 0; i < mipLevels; i++)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = i;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2D[i].extent().x);
			bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2D[i].extent().y);
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

			offset += static_cast<uint32_t>(tex2D[i].size());
		}

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.mipLevels = mipLevels;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.extent = { width, height, 1 };
		imageCreateInfo.usage = imageUsageFlags;

		// Ensure that TRANSFER_DST is set for staging
		if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		CHECK_VK_RESULT(vkCreateImage(device->GetVulkanDevice(), &imageCreateInfo, nullptr, &m_Image));

		vkGetImageMemoryRequirements(device->GetVulkanDevice(), m_Image, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;

		memAllocInfo.memoryTypeIndex = device->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		CHECK_VK_RESULT(vkAllocateMemory(device->GetVulkanDevice(), &memAllocInfo, nullptr, &m_ImageMemory));
		CHECK_VK_RESULT(vkBindImageMemory(device->GetVulkanDevice(), m_Image, m_ImageMemory, 0));

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = mipLevels;
		subresourceRange.layerCount = 1;

		ImageHelpers::SetImageLayout(copyCmd, m_Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer,
			m_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(bufferCopyRegions.size()),
			bufferCopyRegions.data()
		);

		ImageHelpers::SetImageLayout(copyCmd, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageLayout, subresourceRange);

		device->FlushCommandBuffer(copyCmd, queue);
		vkFreeMemory(device->GetVulkanDevice(), stagingMemory, nullptr);
		vkDestroyBuffer(device->GetVulkanDevice(), stagingBuffer, nullptr);

		m_ImageView = ImageHelpers::CreateImageView(m_Image, format, VK_IMAGE_ASPECT_COLOR_BIT);
		CreateTextureSampler();
		UpdateDescriptor();
	}

	void Texture::Cleanup()
	{
		VkDevice device = VulkanRenderer::Get()->GetDevice()->GetVulkanDevice();

		if(m_ImageView)
			vkDestroyImageView(device, m_ImageView, nullptr);
		if(m_Image)
			vkDestroyImage(device, m_Image, nullptr);
		if(m_TextureSampler)
			vkDestroySampler(device, m_TextureSampler, nullptr);
		if(m_ImageMemory)
			vkFreeMemory(device, m_ImageMemory, nullptr);
	}

}