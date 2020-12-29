#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
	class Buffer;
	class DescriptorSet
	{
	public:
		enum class BindingUsage
		{
			VertexShader,
			FragmentShader
		};

		static DescriptorSetRef CreateDescriptorSet(DescriptorPoolRef descPool, DescriptorSetLayoutRef layout);

		DescriptorSet(DescriptorPoolRef descPool, DescriptorSetLayoutRef layout);
		~DescriptorSet();

		struct TextureUniform
        {
		    VkSampler m_Sampler;
		    VkImageView m_ImageView;
        };

		void SetTextureUniform(std::string name, std::vector<TextureUniform> textures, bool isDepth);
		void SetBufferUniform(std::string name, Buffer* buffer);

		void Build();

		VkDescriptorSet& GetVulkanDescriptorSet() { return m_DescriptorSet; }

	private:
		DescriptorPoolRef m_Pool;
		DescriptorSetLayoutRef m_Layout;
		VkDescriptorSet m_DescriptorSet;

		class BindingValue {};

		class TextureBindingValue : public BindingValue 
		{
		public:
			std::vector<TextureUniform> m_Textures;
			bool isDepth;
		};

		class BufferBindingValue : public BindingValue 
		{
		public:
			Buffer* buffer;
		};

		std::unordered_map<std::string, BindingValue*> m_BindingValues;
	};
}