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
		, m_BindingValues()
	{
		for(DescriptorBinding& binding : m_Layout->GetBindings())
		{
			switch (binding.m_Type)
			{
				case DescriptorBindingType::UniformBuffer:
				{
					BufferBindingValue* value = new BufferBindingValue(); 
					value->buffer = nullptr;
					m_BindingValues[binding.m_Name] = value;
					break;
				}
				case DescriptorBindingType::ImageSampler:
				{
					TextureBindingValue* value = new TextureBindingValue();
					value->isDepth = false;
					m_BindingValues[binding.m_Name] = value;
					break;
				}
			}
		}
	}

	DescriptorSet::~DescriptorSet()
	{
		m_Pool->FreeDescriptorSet(this);

		for (auto& [_, bindingValue] : m_BindingValues)
		{
			delete bindingValue;
		}
		m_BindingValues.clear();
	}
	
	void DescriptorSet::SetTextureUniform(std::string name, std::vector<TextureUniform> textures, bool isDepth)
	{
		if (PL_VERIFY(m_BindingValues.count(name) > 0))
		{
			TextureBindingValue* textureBinding = static_cast<TextureBindingValue*>(m_BindingValues[name]);
			textureBinding->m_Textures = textures;
			textureBinding->isDepth = isDepth;
		}
	}
	
	void DescriptorSet::SetBufferUniform(std::string name, Buffer* buffer) 
	{
		if (PL_VERIFY(m_BindingValues.count(name) > 0))
		{
			BufferBindingValue* bindingValue = static_cast<BufferBindingValue*>(m_BindingValues[name]);
			bindingValue->buffer = buffer;
			m_BindingValues[name] = bindingValue;
		}
	}

	void DescriptorSet::Build()
	{
		if (m_DescriptorSet == VK_NULL_HANDLE)
		{
			m_Pool->AllocateDescriptorSet(this, m_Layout.get());
		}

		std::vector<VkWriteDescriptorSet> writeSets;

		//eed to keep these alive until after we update the descriptor set.
		std::vector<VkDescriptorImageInfo> imageInfoList;
		int numImageBindings = 0;
        for (const DescriptorBinding& binding : m_Layout->GetBindings())
        {
            switch (binding.m_Type)
            {
                case DescriptorBindingType::ImageSampler:
                {
                    const TextureBindingValue* textureBinding = static_cast<const TextureBindingValue*>(m_BindingValues[binding.m_Name]);
                    numImageBindings += textureBinding->m_Textures.size();
                    break;
                }
                default: break;
            }
        }
		imageInfoList.resize(numImageBindings);
		int imageIndex = 0;
		for (const DescriptorBinding& binding : m_Layout->GetBindings())
		{
			switch (binding.m_Type)
			{
				case DescriptorBindingType::ImageSampler:
				{
					const TextureBindingValue* textureBinding = static_cast<const TextureBindingValue*>(m_BindingValues[binding.m_Name]);

					int firstImageIndex = imageIndex;
					for (const TextureUniform& textureUniform : textureBinding->m_Textures)
                    {
                        if (textureUniform.m_Sampler != VK_NULL_HANDLE)
                        {
                            VkDescriptorImageInfo &imageInfo = imageInfoList[imageIndex];
                            imageInfo.sampler = textureUniform.m_Sampler;
                            imageInfo.imageView = textureUniform.m_ImageView;
                            imageInfo.imageLayout = textureBinding->isDepth
                                                    ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                                                    : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        }

                        imageIndex++;
                    }

                    VkWriteDescriptorSet writeSet{};
                    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writeSet.dstSet = m_DescriptorSet;
                    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    writeSet.dstBinding = binding.m_Location;
                    writeSet.pImageInfo = &imageInfoList[firstImageIndex];
                    writeSet.descriptorCount = textureBinding->m_Textures.size();

                    writeSets.push_back(writeSet);
					break;
				}
				case DescriptorBindingType::UniformBuffer:
				{
					const BufferBindingValue* bufferBinding = static_cast<const BufferBindingValue*>(m_BindingValues[binding.m_Name]);

					if(bufferBinding->buffer != nullptr)
					{
						VkDescriptorBufferInfo& bufferInfo = bufferBinding->buffer->m_Descriptor;

						VkWriteDescriptorSet writeSet{};
						writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						writeSet.dstSet = m_DescriptorSet;
						writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						writeSet.dstBinding = binding.m_Location;
						writeSet.pBufferInfo = &bufferInfo;
						writeSet.descriptorCount = 1;

						writeSets.push_back(writeSet);
					}
					break;
				}
				default:
				{
					PL_ASSERT(false, "Unhandled PendingDescriptorBindingType in plumbus::vk::DescriptorSet::Build");
				}
			}
		}

		vkUpdateDescriptorSets(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, NULL);
	}
}