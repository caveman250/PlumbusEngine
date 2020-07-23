#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
	struct VertexDescription
	{
		bool m_Valid = false;
		VkPipelineVertexInputStateCreateInfo m_InputState;
		std::vector<VkVertexInputBindingDescription> m_BindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions;
	};

	class Pipeline
	{
	public:
		static PipelineRef CreatePipeline(PipelineLayoutRef pipelineLayout, int numOutputs, VertexDescription vertexDescription, VkPipelineShaderStageCreateInfo vertShader, VkPipelineShaderStageCreateInfo fragShader, VkRenderPass renderPass = VK_NULL_HANDLE);

		Pipeline(PipelineLayoutRef pipelineLayout, int numOutputs, VertexDescription vertexDescription, VkPipelineShaderStageCreateInfo vertShader, VkPipelineShaderStageCreateInfo fragShader, VkRenderPass renderPass = VK_NULL_HANDLE);
		~Pipeline();

		const VkPipeline& GetVulkanPipeline() { return m_Pipeline; }
	private:
		VkPipeline m_Pipeline;
	};
}