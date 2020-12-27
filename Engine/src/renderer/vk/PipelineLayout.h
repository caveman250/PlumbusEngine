#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
	enum class PushConstantUsage
	{
		VertexShader,
		FragmentShader
	};
	
	struct PushConstant
	{
		PushConstantUsage m_Usage;
		uint32_t m_Offset;
		uint32_t m_Size;
	};
	
	class PipelineLayout
	{
	public:
		
		static PipelineLayoutRef CreatePipelineLayout(DescriptorSetLayoutRef layout, std::vector<PushConstant> pushConstants);

		PipelineLayout(DescriptorSetLayoutRef layout, std::vector<PushConstant> pushConstants);
		~PipelineLayout();

		const VkPipelineLayout& GetVulkanPipelineLayout() const { return m_Layout; }

	private:
		VkPipelineLayout m_Layout;
	};
}
