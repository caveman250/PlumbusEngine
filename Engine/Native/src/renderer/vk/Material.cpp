#include "plumbus.h"

#include "renderer/vk/Material.h"
#include "renderer/vk/VulkanRenderer.h"
#include "BaseApplication.h"
#include "DescriptorSetLayout.h"
#include "PipelineCache.h"
#include "PipelineLayout.h"
#include "ShadowManager.h"
#include "shader_compiler/ShaderSettings.h"

namespace plumbus::vk
{
	Material::Material(const char* vertShader, const char* fragShader, VkRenderPass renderPass, bool enableAlphaBlending)
		: m_VertShaderName(vertShader)
		, m_FragShaderName(fragShader)
		, m_Pipeline(VK_NULL_HANDLE)
		, m_PipelineLayout(VK_NULL_HANDLE)
		, m_RenderPass(renderPass)
		, m_ShadersLoaded(false)
		, m_EnableAlphaBlending(enableAlphaBlending)
		, m_CullMode(VK_CULL_MODE_BACK_BIT)
	{
	}
	
	Material::~Material() 
	{
		VkDevice device = VulkanRenderer::Get()->GetDevice()->GetVulkanDevice();

		m_DescriptorSetLayout.reset();
		m_Pipeline.reset();
		m_PipelineLayout.reset();		
	}

	void Material::Setup()
	{
		ShaderReflectionObject shaderReflection;
		if (!m_ShadersLoaded)
		{
			VulkanRenderer* renderer = VulkanRenderer::Get();

			m_VertShaderPipelineCreateInfo = renderer->LoadShader(m_VertShaderName, VK_SHADER_STAGE_VERTEX_BIT, m_ShaderSettings, shaderReflection);
			m_FragShaderPipelineCreateInfo = renderer->LoadShader(m_FragShaderName, VK_SHADER_STAGE_FRAGMENT_BIT, m_ShaderSettings, shaderReflection);
			m_ShadersLoaded = true;
		}

		CreateVertexDescriptions(shaderReflection);

		if (m_PipelineLayout == VK_NULL_HANDLE)
		{
			CreatePipelineLayout(shaderReflection);
		}

		if (!m_Pipeline)
		{
			m_Pipeline = Pipeline::CreatePipeline(m_PipelineLayout, shaderReflection.m_FragmentStageOutputCount, m_VertexDescriptions, m_VertShaderPipelineCreateInfo, m_FragShaderPipelineCreateInfo, m_RenderPass, m_EnableAlphaBlending, m_CullMode);
		}
	}

	void Material::CreatePipelineLayout(const ShaderReflectionObject& shaderReflection)
	{
		m_DescriptorSetLayout = DescriptorSetLayout::CreateDescriptorSetLayout();
		for (const DescriptorBinding& binding : shaderReflection.m_Bindings)
		{
			m_DescriptorSetLayout->AddBinding(binding);
		}
		m_DescriptorSetLayout->Build();

		m_PipelineLayout = PipelineLayout::CreatePipelineLayout(m_DescriptorSetLayout, shaderReflection.m_PushConstants);
	}

	void Material::CreateVertexDescriptions(const ShaderReflectionObject& shaderReflection)
	{
		int stride = 0;
		for (const StageInput& input : shaderReflection.m_VertexStageInputs)
		{
			stride += input.m_Size;
		}
		
		// Binding description
		VkVertexInputBindingDescription vInputBindDescription{};
		vInputBindDescription.binding = 0;
		vInputBindDescription.stride = stride;
		vInputBindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		m_VertexDescriptions.m_BindingDescriptions.resize(1);
		m_VertexDescriptions.m_BindingDescriptions[0] = vInputBindDescription;

		int currOffset = 0;
		//shaderReflection
		for (const StageInput& stageInput : shaderReflection.m_VertexStageInputs)
		{
			VkVertexInputAttributeDescription input{};
			input.location = stageInput.m_Location;
			input.binding = stageInput.m_Binding;
			input.format = stageInput.m_Format;
			input.offset = currOffset;
			m_VertexDescriptions.m_AttributeDescriptions.push_back(input);
			currOffset += stageInput.m_Size;
		}

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
		pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		m_VertexDescriptions.m_InputState = pipelineVertexInputStateCreateInfo;
		m_VertexDescriptions.m_InputState.vertexBindingDescriptionCount = static_cast<uint32_t>(m_VertexDescriptions.m_BindingDescriptions.size());
		m_VertexDescriptions.m_InputState.pVertexBindingDescriptions = m_VertexDescriptions.m_BindingDescriptions.data();
		m_VertexDescriptions.m_InputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_VertexDescriptions.m_AttributeDescriptions.size());
		m_VertexDescriptions.m_InputState.pVertexAttributeDescriptions = m_VertexDescriptions.m_AttributeDescriptions.data();

		m_VertexDescriptions.m_Valid = true;
	}
}
