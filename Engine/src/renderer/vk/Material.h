#pragma once

#include "plumbus.h"

#include "DescriptorSetLayout.h"
#include "Pipeline.h"

namespace plumbus::vk
{
	enum class VertexLayoutComponent
	{
		Position = 0x0,
		Normal = 0x1,
		Colour = 0x2,
		UV = 0x3,
		Tangent = 0x4,
		Bitangent = 0x5,
		DummyFloat = 0x6,
		DummyVec4 = 0x7
	};

	struct VertexLayout
	{
	public:
		std::vector<VertexLayoutComponent> components;

		VertexLayout()
		{
		}

		VertexLayout(std::vector<VertexLayoutComponent> components)
		{
			this->components = std::move(components);
		}

		uint32_t stride()
		{
			uint32_t res = 0;
			for (auto& component : components)
			{
				switch (component)
				{
				case VertexLayoutComponent::UV:
					res += 2 * sizeof(float);
					break;
				case VertexLayoutComponent::DummyFloat:
					res += sizeof(float);
					break;
				case VertexLayoutComponent::DummyVec4:
					res += 4 * sizeof(float);
					break;
				default:
					// All components except the ones listed above are made up of 3 floats
					res += 3 * sizeof(float);
				}
			}
			return res;
		}
	};

	class Material
	{
	public:
		Material(const char* vertShader, const char* fragShader, VkRenderPass renderPass = VK_NULL_HANDLE);
		~Material();
		virtual void Setup(VertexLayout layout);
		const PipelineRef& GetPipeline() { return m_Pipeline; }
		const PipelineLayoutRef& GetPipelineLayout() { return m_PipelineLayout; }

		const DescriptorSetLayoutRef& GetLayout() { return m_DescriptorSetLayout; }

	private:
		void CreatePipelineLayout(const std::vector<DescriptorBinding>& binding);
		void CreateVertexDescriptions();

		VertexDescription m_VertexDescriptions;
		DescriptorSetLayoutRef m_DescriptorSetLayout;

		PipelineLayoutRef m_PipelineLayout;
		PipelineRef m_Pipeline;

		VertexLayout m_VertexLayout;
		const char* m_VertShaderName;
		const char* m_FragShaderName;

		VkRenderPass m_RenderPass;

		VkPipelineShaderStageCreateInfo m_VertShaderPipelineCreateInfo;
		VkPipelineShaderStageCreateInfo m_FragShaderPipelineCreateInfo;
		bool m_ShadersLoaded;
	};
}