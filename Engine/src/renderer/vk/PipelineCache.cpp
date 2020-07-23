#include "PipelineCache.h"
#include "VulkanRenderer.h"

namespace plumbus::vk
{
	PipelineCacheRef PipelineCache::CreatePipelineCache()
	{
		return std::make_shared<PipelineCache>();
	}

	PipelineCache::PipelineCache()
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		CHECK_VK_RESULT(vkCreatePipelineCache(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &pipelineCacheCreateInfo, nullptr, &m_Cache));
	}

	PipelineCache::~PipelineCache()
	{
		vkDestroyPipelineCache(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), m_Cache, nullptr);
	}

}