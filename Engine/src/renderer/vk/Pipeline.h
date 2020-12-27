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

	struct StageInput
	{
		uint32_t m_Location;
		uint32_t m_Binding;
		uint32_t m_Size;
		VkFormat m_Format;
		
		StageInput& operator =(const StageInput& other)
		{
			m_Location = other.m_Location;
			m_Binding = other.m_Binding;
			m_Size = other.m_Size;
			m_Format = other.m_Format;
			return *this;
		}
	};

	class Pipeline
	{
	public:
		static PipelineRef CreatePipeline(PipelineLayoutRef pipelineLayout, int numOutputs, VertexDescription vertexDescription, VkPipelineShaderStageCreateInfo vertShader, VkPipelineShaderStageCreateInfo fragShader, VkRenderPass renderPass = VK_NULL_HANDLE, bool enableAlphaBlending = false);

		Pipeline(PipelineLayoutRef pipelineLayout, int numOutputs, VertexDescription vertexDescription, VkPipelineShaderStageCreateInfo vertShader, VkPipelineShaderStageCreateInfo fragShader, VkRenderPass renderPass = VK_NULL_HANDLE, bool enableAlphaBlending = false);
		~Pipeline();

		const VkPipeline& GetVulkanPipeline() { return m_Pipeline; }
	private:
		VkPipeline m_Pipeline;
	};
}