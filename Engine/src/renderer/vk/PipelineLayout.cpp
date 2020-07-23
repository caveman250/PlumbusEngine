#include "PipelineLayout.h"
#include "VulkanRenderer.h"

namespace plumbus::vk
{
	PipelineLayoutRef PipelineLayout::CreatePipelineLayout(DescriptorSetLayoutRef layout)
	{
		return std::make_shared<PipelineLayout>(layout);
	}

	PipelineLayout::PipelineLayout(DescriptorSetLayoutRef layout)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &layout->GetVulkanDescriptorSetLayout();

		CHECK_VK_RESULT(vkCreatePipelineLayout(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &pipelineLayoutCreateInfo, nullptr, &m_Layout));
	}

	PipelineLayout::~PipelineLayout()
	{
 		vkDestroyPipelineLayout(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), m_Layout, nullptr);
	}
}