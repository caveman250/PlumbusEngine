#include "PipelineLayout.h"
#include "VulkanRenderer.h"

namespace plumbus::vk
{
	PipelineLayoutRef PipelineLayout::CreatePipelineLayout(DescriptorSetLayoutRef layout, std::vector<PushConstant> pushConstants)
	{
		return std::make_shared<PipelineLayout>(layout, pushConstants);
	}

	PipelineLayout::PipelineLayout(DescriptorSetLayoutRef layout, std::vector<PushConstant> pushConstants)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;

		std::vector<VkPushConstantRange> pushConstantRanges;
		if (pushConstants.size() > 0)
		{
			for (const PushConstant& pushConstant : pushConstants)
			{
				VkPushConstantRange pushConstantRange{};
				pushConstantRange.stageFlags =  pushConstant.m_Usage == PushConstantUsage::VertexShader ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
				pushConstantRange.offset = pushConstant.m_Offset;
				pushConstantRange.size = pushConstant.m_Size;
				pushConstantRanges.push_back(pushConstantRange);
			}

			pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRanges.size();
			pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
		}
		pipelineLayoutCreateInfo.pSetLayouts = &layout->GetVulkanDescriptorSetLayout();

		CHECK_VK_RESULT(vkCreatePipelineLayout(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &pipelineLayoutCreateInfo, nullptr, &m_Layout));
	}

	PipelineLayout::~PipelineLayout()
	{
 		vkDestroyPipelineLayout(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), m_Layout, nullptr);
	}
}