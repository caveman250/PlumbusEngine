#include "plumbus.h"

#include "renderer/vk/Material.h"
#include "renderer/vk/VulkanRenderer.h"
#include "BaseApplication.h"
#include "DescriptorSetLayout.h"

namespace plumbus::vk
{
	Material::Material(const char* vertShader, const char* fragShader)
		: base::Material(vertShader, fragShader)
		, m_VertShaderName(vertShader)
		, m_FragShaderName(fragShader)
		, m_Pipeline(VK_NULL_HANDLE)
		, m_PipelineLayout(VK_NULL_HANDLE)
		, m_ShadersLoaded(false)
	{
	}

	void Material::Setup(VertexLayout* layout)
	{
		m_VertexLayout = *layout;
		CreateVertexDescriptions();

		std::vector<DescriptorSetLayout::Binding> bindingInfo;
		if (!m_ShadersLoaded)
		{
			VulkanRenderer* renderer = VulkanRenderer::Get();
			m_VertShaderPipelineCreateInfo = renderer->LoadShader(m_VertShaderName, VK_SHADER_STAGE_VERTEX_BIT, bindingInfo);
			m_FragShaderPipelineCreateInfo = renderer->LoadShader(m_FragShaderName, VK_SHADER_STAGE_FRAGMENT_BIT, bindingInfo);
		}

		if(m_PipelineLayout == VK_NULL_HANDLE)
		{
			CreatePipelineLayout(bindingInfo);
		}

		if(m_Pipeline == VK_NULL_HANDLE)
		{
			CreatePipeline();
		}
	}

	void Material::Destroy()
	{
		vk::VulkanRenderer* renderer = VulkanRenderer::Get();

		m_DescriptorSetLayout.reset();
		vkDestroyPipeline(renderer->GetDevice()->GetVulkanDevice(), m_Pipeline, nullptr);
		vkDestroyPipelineLayout(renderer->GetDevice()->GetVulkanDevice(), m_PipelineLayout, nullptr);
	}

	void Material::CreatePipelineLayout(const std::vector<DescriptorSetLayout::Binding>& bindings)
	{
		m_DescriptorSetLayout = DescriptorSetLayout::CreateDescriptorSetLayout();
		for (const DescriptorSetLayout::Binding& binding : bindings)
		{
			m_DescriptorSetLayout->AddBinding(binding);
		}
		m_DescriptorSetLayout->Build();

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout->GetVulkanDescriptorSetLayout();

		CHECK_VK_RESULT(vkCreatePipelineLayout(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));
	}

	void Material::CreatePipeline()
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

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = 0xf;
		blendAttachmentState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;

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
		pipelineCreateInfo.layout = m_PipelineLayout;
		pipelineCreateInfo.renderPass = renderer->GetOffscreenFramebuffer()->GetRenderPass();
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

		// Offscreen pipeline
		pipelineCreateInfo.pVertexInputState = &m_VertexDescriptions.m_InputState;

		shaderStages[0] = m_VertShaderPipelineCreateInfo;
		shaderStages[1] = m_FragShaderPipelineCreateInfo;

		// Blend attachment states required for all color attachments
		// This is important, as color write mask will otherwise be 0x0 and you
		// won't see anything rendered to the attachment
		VkPipelineColorBlendAttachmentState blendAttachmentState1{};
		blendAttachmentState1.colorWriteMask = 0xf;
		blendAttachmentState1.blendEnable = VK_FALSE;
		VkPipelineColorBlendAttachmentState blendAttachmentState2{};
		blendAttachmentState2.colorWriteMask = 0xf;
		blendAttachmentState2.blendEnable = VK_FALSE;
		VkPipelineColorBlendAttachmentState blendAttachmentState3{};
		blendAttachmentState3.colorWriteMask = 0xf;
		blendAttachmentState3.blendEnable = VK_FALSE;

		std::array<VkPipelineColorBlendAttachmentState, 3> blendAttachmentStates = {
			blendAttachmentState1,
			blendAttachmentState2,
			blendAttachmentState3
		};

		colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
		colorBlendState.pAttachments = blendAttachmentStates.data();

		CHECK_VK_RESULT(vkCreateGraphicsPipelines(renderer->GetDevice()->GetVulkanDevice(), renderer->GetPipelineCache(), 1, &pipelineCreateInfo, nullptr, &m_Pipeline));
	}

	void Material::CreateVertexDescriptions()
	{
		// Binding description
		VkVertexInputBindingDescription vInputBindDescription{};
		vInputBindDescription.binding = 0;
		vInputBindDescription.stride = m_VertexLayout.stride();
		vInputBindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		m_VertexDescriptions.m_BindingDescriptions.resize(1);
		m_VertexDescriptions.m_BindingDescriptions[0] = vInputBindDescription;

		// Attribute descriptions
		m_VertexDescriptions.m_AttributeDescriptions.resize(m_VertexLayout.components.size());

		int currOffset = 0;
		for (int i = 0; i < m_VertexLayout.components.size(); ++i)
		{
			VkVertexInputAttributeDescription input{};
			input.location = i;
			input.binding = 0;
			input.format = VK_FORMAT_R32G32B32_SFLOAT;
			input.offset = sizeof(float) * currOffset;
			m_VertexDescriptions.m_AttributeDescriptions[i] = input;

			switch (m_VertexLayout.components[i])
			{
			case VERTEX_COMPONENT_POSITION:
				currOffset += 3;
				break;
			case VERTEX_COMPONENT_NORMAL:
				currOffset += 3;
				break;
			case VERTEX_COMPONENT_COLOR:
				currOffset += 3;
				break;
			case VERTEX_COMPONENT_UV:
				currOffset += 2;
				break;
			case VERTEX_COMPONENT_TANGENT:
				currOffset += 3;
				break;
			case VERTEX_COMPONENT_BITANGENT:
				currOffset += 3;
				break;
			default:
				Log::Error("Material::CreateVertexDescriptions unsupported VertexLayoutComponent");
				break;
			}
		}

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
		pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		m_VertexDescriptions.m_InputState = pipelineVertexInputStateCreateInfo;
		m_VertexDescriptions.m_InputState.vertexBindingDescriptionCount = static_cast<uint32_t>(m_VertexDescriptions.m_BindingDescriptions.size());
		m_VertexDescriptions.m_InputState.pVertexBindingDescriptions = m_VertexDescriptions.m_BindingDescriptions.data();
		m_VertexDescriptions.m_InputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_VertexDescriptions.m_AttributeDescriptions.size());
		m_VertexDescriptions.m_InputState.pVertexAttributeDescriptions = m_VertexDescriptions.m_AttributeDescriptions.data();
	}
}