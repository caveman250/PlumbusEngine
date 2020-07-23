#include "DescriptorSetLayout.h"
#include "VulkanRenderer.h"

namespace plumbus::vk
{
    DescriptorSetLayoutRef DescriptorSetLayout::CreateDescriptorSetLayout()
    {
        return std::make_shared<DescriptorSetLayout>();
    }

    DescriptorSetLayout::DescriptorSetLayout()
    {

    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), m_Layout, nullptr);
    }

    void DescriptorSetLayout::AddBinding(DescriptorBinding binding)
    {
        m_PendingBindings.push_back(binding);
    }

    void DescriptorSetLayout::AddBinding(DescriptorBindingUsage usage, DescriptorBindingType type, int location, std::string name)
    {
        DescriptorBinding binding = {};
        binding.m_Usage = usage;
        binding.m_Type = type;
        binding.m_Location = location;
		binding.m_Name = name;
        m_PendingBindings.push_back(binding);
    }

    void DescriptorSetLayout::Build()
    {
        VkDevice device = VulkanRenderer::Get()->GetDevice()->GetVulkanDevice();

		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

		for (const DescriptorBinding& binding : m_PendingBindings)
		{
			switch (binding.m_Type)
			{
				case DescriptorBindingType::ImageSampler:
				{
					VkDescriptorSetLayoutBinding layoutBinding{};
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					layoutBinding.stageFlags = binding.m_Usage == DescriptorBindingUsage::VertexShader ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
					layoutBinding.binding = binding.m_Location;
					layoutBinding.descriptorCount = 1;

					layoutBindings.push_back(layoutBinding);
					break;
				}
				case DescriptorBindingType::UniformBuffer:
				{
					VkDescriptorSetLayoutBinding layoutBinding{};
					layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					layoutBinding.stageFlags = binding.m_Usage == DescriptorBindingUsage::VertexShader ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
					layoutBinding.binding = binding.m_Location;
					layoutBinding.descriptorCount = 1;

					layoutBindings.push_back(layoutBinding);
					break;
				}
				default:
				{
					PL_ASSERT(false, "Unhandled PendingDescriptorBindingType in plumbus::vk::DescriptorSet::Build");
				}
			}
		}

		VkDescriptorSetLayoutCreateInfo descriptorLayout{};
		descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayout.pBindings = layoutBindings.data();
		descriptorLayout.bindingCount = static_cast<uint32_t>(layoutBindings.size());

		CHECK_VK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &m_Layout));
    }
}