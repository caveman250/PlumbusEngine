#include "plumbus.h"

#include "renderer/vk/Material.h"
#include "renderer/vk/VulkanRenderer.h"
#include "BaseApplication.h"
#include "DescriptorSetLayout.h"
#include "PipelineCache.h"
#include "PipelineLayout.h"

namespace plumbus::vk
{
	Material::Material(const char* vertShader, const char* fragShader, VkRenderPass renderPass)
		: m_VertShaderName(vertShader)
		, m_FragShaderName(fragShader)
		, m_Pipeline(VK_NULL_HANDLE)
		, m_PipelineLayout(VK_NULL_HANDLE)
		, m_VertexLayout()
		, m_RenderPass(renderPass)
		, m_ShadersLoaded(false)
	{
	}
	
	Material::~Material() 
	{
		VkDevice device = VulkanRenderer::Get()->GetDevice()->GetVulkanDevice();

		m_DescriptorSetLayout.reset();
		m_Pipeline.reset();
		m_PipelineLayout.reset();		
	}

	void Material::Setup(VertexLayout layout)
	{
		if (layout.components.size() > 0)
		{
			m_VertexLayout = layout;
			CreateVertexDescriptions();
		}

		std::vector<DescriptorBinding> bindingInfo;
		int numVertexOutputs = 0;
		int numFragOutputs = 0;
		if (!m_ShadersLoaded)
		{
			VulkanRenderer* renderer = VulkanRenderer::Get();
			m_VertShaderPipelineCreateInfo = renderer->LoadShader(m_VertShaderName, VK_SHADER_STAGE_VERTEX_BIT, bindingInfo, numVertexOutputs);
			m_FragShaderPipelineCreateInfo = renderer->LoadShader(m_FragShaderName, VK_SHADER_STAGE_FRAGMENT_BIT, bindingInfo, numFragOutputs);
			m_ShadersLoaded = true;
		}

		if(m_PipelineLayout == VK_NULL_HANDLE)
		{
			CreatePipelineLayout(bindingInfo);
		}

		if(!m_Pipeline)
		{
			m_Pipeline = Pipeline::CreatePipeline(m_PipelineLayout, numFragOutputs, m_VertexDescriptions, m_VertShaderPipelineCreateInfo, m_FragShaderPipelineCreateInfo, m_RenderPass);
		}
	}

	void Material::CreatePipelineLayout(const std::vector<DescriptorBinding>& bindings)
	{
		m_DescriptorSetLayout = DescriptorSetLayout::CreateDescriptorSetLayout();
		for (const DescriptorBinding& binding : bindings)
		{
			m_DescriptorSetLayout->AddBinding(binding);
		}
		m_DescriptorSetLayout->Build();

		m_PipelineLayout = PipelineLayout::CreatePipelineLayout(m_DescriptorSetLayout);
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
			case VertexLayoutComponent::Position:
				currOffset += 3;
				break;
			case VertexLayoutComponent::Normal:
				currOffset += 3;
				break;
			case VertexLayoutComponent::Colour:
				currOffset += 3;
				break;
			case VertexLayoutComponent::UV:
				currOffset += 2;
				break;
			case VertexLayoutComponent::Tangent:
				currOffset += 3;
				break;
			case VertexLayoutComponent::Bitangent:
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

		m_VertexDescriptions.m_Valid = true;
	}
}