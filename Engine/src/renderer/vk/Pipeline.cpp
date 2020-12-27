#include "Pipeline.h"
#include "VulkanRenderer.h"
#include "PipelineLayout.h"
#include "PipelineCache.h"

namespace plumbus::vk
{
	PipelineRef Pipeline::CreatePipeline(PipelineLayoutRef pipelineLayout, int numOutputs, VertexDescription vertexDescription, VkPipelineShaderStageCreateInfo vertShader, VkPipelineShaderStageCreateInfo fragShader, VkRenderPass renderPass, bool enableAlphaBlending) 
	{
		return std::make_shared<Pipeline>(pipelineLayout, numOutputs, vertexDescription, vertShader, fragShader, renderPass, enableAlphaBlending);
	}

	Pipeline::Pipeline(PipelineLayoutRef pipelineLayout, int numOutputs, VertexDescription vertexDescription, VkPipelineShaderStageCreateInfo vertShader, VkPipelineShaderStageCreateInfo fragShader, VkRenderPass renderPass, bool enableAlphaBlending)
	{
		vk::VulkanRenderer* renderer = VulkanRenderer::Get();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.flags = 0;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.flags = 0;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		// Enable blending
		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.front = depthStencilState.back;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.flags = 0;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.flags = 0;

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		dynamicState.flags = 0;

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = pipelineLayout->GetVulkanPipelineLayout();
		pipelineCreateInfo.renderPass = renderPass != VK_NULL_HANDLE ? renderPass : VulkanRenderer::Get()->GetDeferredFramebuffer()->GetRenderPass();
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.basePipelineIndex = -1;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();

		VkPipelineVertexInputStateCreateInfo inputState = {};
		if (vertexDescription.m_Valid)
		{
			pipelineCreateInfo.pVertexInputState = &vertexDescription.m_InputState;
		}
		else
		{
			//dummy vertex input state to keep validation happy. 
			inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			inputState.vertexBindingDescriptionCount = 0;
			inputState.pVertexBindingDescriptions = nullptr;
			inputState.vertexAttributeDescriptionCount = 0;
			inputState.pVertexAttributeDescriptions = nullptr;

			pipelineCreateInfo.pVertexInputState = &inputState;
		}

		shaderStages[0] = vertShader;
		shaderStages[1] = fragShader;

		// Blend attachment states required for all color attachments
		// This is important, as color write mask will otherwise be 0x0 and you
		// won't see anything rendered to the attachment

		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;

		for (int i = 0; i < numOutputs; ++i)
		{
			VkPipelineColorBlendAttachmentState blendAttachmentState{};
			if (enableAlphaBlending)
			{
				blendAttachmentState.blendEnable = VK_TRUE;
				blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
				blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
			}
			else
			{
				blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				blendAttachmentState.blendEnable = VK_FALSE;
			}
			
			blendAttachmentStates.push_back(std::move(blendAttachmentState));
		}

		colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
		colorBlendState.pAttachments = blendAttachmentStates.data();

		CHECK_VK_RESULT(vkCreateGraphicsPipelines(renderer->GetDevice()->GetVulkanDevice(), renderer->GetPipelineCache()->GetVulkanPipelineCache(), 1, &pipelineCreateInfo, nullptr, &m_Pipeline));
	}

	Pipeline::~Pipeline()
	{
		vkDestroyPipeline(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), m_Pipeline, nullptr);
	}
}