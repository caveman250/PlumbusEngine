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

		void AddTexture(VkSampler sampler, VkImageView imageView, BindingUsage usage);
		void AddFramebufferAttachment(FrameBufferRef fb, std::string attachmentName, BindingUsage usage);
		void AddBuffer(Buffer* buffer, BindingUsage usage);

		void Build();

		VkDescriptorSet& GetVulkanDescriptorSet() { return m_DescriptorSet; }

	private:
		enum class PendingDescriptorBindingType
		{
			UniformBuffer,
			ImageSampler
		};

		class PendingDescriptorBinding
		{
		public:
			PendingDescriptorBinding(PendingDescriptorBindingType type, BindingUsage usage) : m_Type(type), m_Usage(usage) {}
			PendingDescriptorBindingType m_Type;
			BindingUsage m_Usage;
		};

		class PendingBufferBinding : public PendingDescriptorBinding
		{
		public:
			PendingBufferBinding(BindingUsage usage) : PendingDescriptorBinding(PendingDescriptorBindingType::UniformBuffer, usage), m_BufferInfo() {}
			VkDescriptorBufferInfo m_BufferInfo;
		};

		class PendingSamplerBinding : public PendingDescriptorBinding
		{
		public:
			PendingSamplerBinding(BindingUsage usage) : PendingDescriptorBinding(PendingDescriptorBindingType::ImageSampler, usage), m_ImageInfo() {}
			VkDescriptorImageInfo m_ImageInfo;
		};

		DescriptorPoolRef m_Pool;
		DescriptorSetLayoutRef m_Layout;
		std::vector<PendingDescriptorBinding*> m_Bindings;
		VkDescriptorSet m_DescriptorSet;
	};
}