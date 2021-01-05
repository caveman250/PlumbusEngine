#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
	class PipelineCache
	{
	public:
		static PipelineCacheRef CreatePipelineCache();

		PipelineCache();
		~PipelineCache();

		const VkPipelineCache& GetVulkanPipelineCache() { return m_Cache; }
	private:
		VkPipelineCache m_Cache;
	};
}