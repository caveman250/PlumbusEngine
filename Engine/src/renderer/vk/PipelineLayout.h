#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
	class PipelineLayout
	{
	public:
		static PipelineLayoutRef CreatePipelineLayout(DescriptorSetLayoutRef layout);

		PipelineLayout(DescriptorSetLayoutRef layout);
		~PipelineLayout();

		const VkPipelineLayout& GetVulkanPipelineLayout() { return m_Layout; }

	private:
		VkPipelineLayout m_Layout;
	};
}
