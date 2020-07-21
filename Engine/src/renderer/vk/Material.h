#pragma once

#include "plumbus.h"

#include "renderer/base/Material.h"
#include "DescriptorSetLayout.h"

namespace plumbus::vk
{
	class Material : public base::Material
	{
	public:
		Material(const char* vertShader, const char* fragShader);
		virtual void Setup(VertexLayout* layout) override;
		virtual void Destroy() override;
		VkPipeline& GetPipeline() { return m_Pipeline; }
		VkPipelineLayout& GetPipelineLayout() { return m_PipelineLayout; }

		const DescriptorSetLayoutRef& GetLayout() { return m_DescriptorSetLayout; }

	private:
		void CreatePipelineLayout(const std::vector<DescriptorSetLayout::Binding>& binding);
		void CreatePipeline();
		void CreateVertexDescriptions();

		struct VertexDescription
		{
			VkPipelineVertexInputStateCreateInfo m_InputState;
			std::vector<VkVertexInputBindingDescription> m_BindingDescriptions;
			std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions;
		};

		VertexDescription m_VertexDescriptions;
		DescriptorSetLayoutRef m_DescriptorSetLayout;

		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;

		VertexLayout m_VertexLayout;
		const char* m_VertShaderName;
		const char* m_FragShaderName;

		VkPipelineShaderStageCreateInfo m_VertShaderPipelineCreateInfo;
		VkPipelineShaderStageCreateInfo m_FragShaderPipelineCreateInfo;
		bool m_ShadersLoaded;
	};
}