#include "plumbus.h"

#include "renderer/vk/Texture.h"
#include "BaseApplication.h"
#include "gli/gli.hpp"
#include "renderer/vk/ImageHelpers.h"

namespace plumbus::vk
{
	void Texture::CreateTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_FALSE;
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

	struct ASTCHeader
	{
		/// Magic value
		uint8_t magic[4];
		/// Block size in X
		uint8_t blockdimX;
		/// Block size in Y
		uint8_t blockdimY;
		/// Block size in Z
		uint8_t blockdimZ;
		/// Size of the image in pixels (X), least significant byte first.
		uint8_t xsize[3];
		/// Size of the image in pixels (Y), least significant byte first.
		uint8_t ysize[3];
		/// Size of the image in pixels (Z), least significant byte first.
		uint8_t zsize[3];
	};

	struct ASTCTexture
	{
		void* data() { return m_Buffer.data(); }
		int size() { return m_Buffer.size(); }

		std::vector<char> m_Buffer;
		unsigned int m_Width = 0;
		unsigned int m_Height = 0;
		VkFormat m_Format = VK_FORMAT_UNDEFINED;
	};


#define ASTC_MAGIC 0x5CA1AB13

	ASTCTexture LoadASTCTexture(std::vector<char> buffer)
	{
		ASTCHeader header;
		memcpy(&header, buffer.data(), sizeof(ASTCHeader));
		uint32_t magic = header.magic[0] | (uint32_t(header.magic[1]) << 8) | (uint32_t(header.magic[2]) << 16) |
						 (uint32_t(header.magic[3]) << 24);

		if (magic != ASTC_MAGIC)
		{
			Log::Error("Texture is not astc");
			return ASTCTexture();
		}

		if (header.blockdimZ != 1)
		{
			Log::Error("3d ASTC textures not supported");
			return ASTCTexture();
		}

		ASTCTexture texture;

		if (header.blockdimX == 4 && header.blockdimY == 4) // 4x4
			texture.m_Format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
		else if (header.blockdimX == 5 && header.blockdimY == 4) // 5x4
			texture.m_Format = VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
		else if (header.blockdimX == 6 && header.blockdimY == 5) // 6x5
			texture.m_Format = VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
		else if (header.blockdimX == 6 && header.blockdimY == 6) // 6x6
			texture.m_Format = VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
		else if (header.blockdimX == 8 && header.blockdimY == 5) // 8x5
			texture.m_Format = VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
		else if (header.blockdimX == 8 && header.blockdimY == 6) // 8x6
			texture.m_Format = VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
		else if (header.blockdimX == 8 && header.blockdimY == 8) // 8x8
			texture.m_Format = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
		else if (header.blockdimX == 10 && header.blockdimY == 5) // 10x5
			texture.m_Format = VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
		else if (header.blockdimX == 10 && header.blockdimY == 6) // 10x6
			texture.m_Format = VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
		else if (header.blockdimX == 10 && header.blockdimY == 8) // 10x8
			texture.m_Format = VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
		else if (header.blockdimX == 10 && header.blockdimY == 10) // 10x10
			texture.m_Format = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
		else if (header.blockdimX == 12 && header.blockdimY == 10) // 12x10
			texture.m_Format = VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
		else if (header.blockdimX == 12 && header.blockdimY == 12) // 12x12
			texture.m_Format = VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
		else
		{
			Log::Error("Unknown ASTC block size %u x %u.\n", header.blockdimX, header.blockdimY);
			return ASTCTexture();
		}


		texture.m_Buffer.insert(end(texture.m_Buffer), begin(buffer) + sizeof(ASTCHeader), end(buffer));
		texture.m_Width = header.xsize[0] | (header.xsize[1] << 8) | (header.xsize[2] << 16);
		texture.m_Height = header.ysize[0] | (header.ysize[1] << 8) | (header.ysize[2] << 16);
		return texture;
	}

	void Texture::LoadTexture(std::string filename)
	{
		VkFormat format = VK_FORMAT_BC3_UNORM_BLOCK;
		VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkQueue queue = VulkanRenderer::Get()->GetDevice()->GetGraphicsQueue();

		std::shared_ptr<vk::Device> device = VulkanRenderer::Get()->GetDevice();

#if PL_PLATFORM_ANDROID
        std::vector<char> fileContents = Helpers::ReadBinaryFile(filename);
		ASTCTexture tex2D = LoadASTCTexture(fileContents);

		uint32_t width = static_cast<uint32_t>(tex2D.m_Width);
		uint32_t height = static_cast<uint32_t>(tex2D.m_Height);
		uint32_t mipLevels = 1;
		format = tex2D.m_Format;
#else
		gli::texture2d tex2D(gli::load(filename));

		PL_ASSERT(!tex2D.empty());

		uint32_t width = static_cast<uint32_t>(tex2D[0].extent().x);
		uint32_t height = static_cast<uint32_t>(tex2D[0].extent().y);
		uint32_t mipLevels = static_cast<uint32_t>(tex2D.levels());
#endif

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
#if PL_PLATFORM_ANDROID
			bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2D.m_Width);
			bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2D.m_Height);
#else
			bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2D[i].extent().x);
			bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2D[i].extent().y);
#endif
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

#if PL_PLATFORM_ANDROID
			offset += static_cast<uint32_t>(tex2D.size());
#else
			offset += static_cast<uint32_t>(tex2D[i].size());
#endif
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