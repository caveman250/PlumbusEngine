#include "DescriptorSet.h"
#include "Buffer.h"
#include "VulkanRenderer.h"
#include "DescriptorPool.h"

namespace plumbus::vk
{
	plumbus::vk::DescriptorSetRef DescriptorSet::CreateDescriptorSet(DescriptorPoolRef descPool, DescriptorSetLayoutRef layout)
	{
		return std::make_shared<DescriptorSet>(descPool, layout);
	}

	DescriptorSet::DescriptorSet(DescriptorPoolRef descPool, DescriptorSetLayoutRef layout)
		: m_Pool(descPool)
		, m_Layout(layout)
		, m_DescriptorSet()
		, m_Bindings()
	{

	}

	DescriptorSet::~DescriptorSet()
	{
		VkDevice device = VulkanRenderer::Get()->GetDevice()->GetVulkanDevice();
		m_Pool->FreeDescriptorSet(this);

		for (PendingDescriptorBinding* binding : m_Bindings)
		{
			delete binding;
		}
		m_Bindings.clear();
	}

	void DescriptorSet::AddBuffer(Buffer* buffer, BindingUsage usage)
	{
		m_Bindings.push_back(new PendingBufferBinding(usage));
		static_cast<PendingBufferBinding*>(m_Bindings.back())->m_BufferInfo = buffer->m_Descriptor;
	}

	void DescriptorSet::AddTexture(VkSampler sampler, VkImageView imageView, BindingUsage usage)
	{
		m_Bindings.push_back(new PendingSamplerBinding(usage));
		PendingSamplerBinding* binding = static_cast<PendingSamplerBinding*>(m_Bindings.back());
		binding->m_ImageInfo.sampler = sampler;
		binding->m_ImageInfo.imageView = imageView;
		binding->m_ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	void DescriptorSet::AddFramebufferAttachment(FrameBufferRef fb, std::string attachmentName, BindingUsage usage)
	{
		AddTexture(fb->GetSampler(), fb->GetAttachment(attachmentName).m_ImageView, usage);
	}


	void DescriptorSet::Build()
	{
		VkDevice device = VulkanRenderer::Get()->GetDevice()->GetVulkanDevice();

		m_Pool->AllocateDescriptorSet(this, m_Layout.get());

		std::vector<VkWriteDescriptorSet> writeSets;
		int bindingIndex = 0;
		for (const PendingDescriptorBinding* binding : m_Bindings)
		{
			switch (binding->m_Type)
			{
				case PendingDescriptorBindingType::ImageSampler:
				{
					const PendingSamplerBinding* samplerBinding = static_cast<const PendingSamplerBinding*>(binding);

					VkWriteDescriptorSet writeSet {};
					writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeSet.dstSet = m_DescriptorSet;
					writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					writeSet.dstBinding = bindingIndex;
					writeSet.pImageInfo = &samplerBinding->m_ImageInfo;
					writeSet.descriptorCount = 1;

					writeSets.push_back(writeSet);
					break;
				}
				case PendingDescriptorBindingType::UniformBuffer:
				{
					const PendingBufferBinding* bufferBinding = static_cast<const PendingBufferBinding*>(binding);

					VkWriteDescriptorSet writeSet{};
					writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeSet.dstSet = m_DescriptorSet;
					writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					writeSet.dstBinding = bindingIndex;
					writeSet.pBufferInfo = &bufferBinding->m_BufferInfo;
					writeSet.descriptorCount = 1;

					writeSets.push_back(writeSet);
					break;
				}
				default:
				{
					PL_ASSERT(false, "Unhandled PendingDescriptorBindingType in plumbus::vk::DescriptorSet::Build");
				}
			}

			bindingIndex++;
		}

		vkUpdateDescriptorSets(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, NULL);
	}
}