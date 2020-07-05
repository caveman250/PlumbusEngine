#pragma once

#include "plumbus.h"

#include "renderer/base/Material.h"

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
	private:
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateVertexDescriptions();

		struct VertexDescription
		{
			VkPipelineVertexInputStateCreateInfo m_InputState;
			std::vector<VkVertexInputBindingDescription> m_BindingDescriptions;
			std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions;
		};

		VertexDescription m_VertexDescriptions;
		VkDescriptorSetLayout m_DescriptorSetLayout;

		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;

		VertexLayout m_VertexLayout;
		const char* m_VertShaderName;
		const char* m_FragShaderName;
	};
}