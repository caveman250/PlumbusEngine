#pragma once

#include "plumbus.h"

#include "DescriptorSetLayout.h"
#include "Pipeline.h"
#include "shader_compiler/ShaderSettings.h"

namespace plumbus::vk
{
	struct PushConstant;
	struct ShaderReflectionObject;

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

	class Material
	{
	public:
		Material(const char* vertShader, const char* fragShader, VkRenderPass renderPass = VK_NULL_HANDLE, bool enableAlphaBlending = false);
		~Material();
		virtual void Setup();
		const PipelineRef& GetPipeline() { return m_Pipeline; }
		const PipelineLayoutRef& GetPipelineLayout() { return m_PipelineLayout; }

		const DescriptorSetLayoutRef& GetLayout() { return m_DescriptorSetLayout; }
		shaders::ShaderSettings& GetShaderSettings() { return m_ShaderSettings; }

	private:
		void CreatePipelineLayout(const ShaderReflectionObject& shaderReflection);
		void CreateVertexDescriptions(const ShaderReflectionObject& shaderReflection);

		VertexDescription m_VertexDescriptions;
		DescriptorSetLayoutRef m_DescriptorSetLayout;

		bool m_EnableAlphaBlending;
		PipelineLayoutRef m_PipelineLayout;
		PipelineRef m_Pipeline;

		const char* m_VertShaderName;
		const char* m_FragShaderName;

		VkRenderPass m_RenderPass;

		VkPipelineShaderStageCreateInfo m_VertShaderPipelineCreateInfo;
		VkPipelineShaderStageCreateInfo m_FragShaderPipelineCreateInfo;
		bool m_ShadersLoaded;

		shaders::ShaderSettings m_ShaderSettings;
	};
}
