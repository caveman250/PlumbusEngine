#include "Model.h"
#include "Application.h"
#include <chrono>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "ModelManager.h"
#include "Scene.h"

Model::~Model()
{
	Application& app = Application::Get();
	vkDestroyImage(app.GetDevice(), m_Image, nullptr);
	vkFreeMemory(app.GetDevice(), m_ImageMemory, nullptr);

	vkDestroySampler(app.GetDevice(), m_TextureSampler, nullptr);
	vkDestroyImageView(app.GetDevice(), m_ImageView, nullptr);
}

void Model::CreateTextureSampler()
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
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(Application::Get().GetDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}

