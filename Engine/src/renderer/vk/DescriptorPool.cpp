#include "DescriptorPool.h"
#include "VulkanRenderer.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"

namespace plumbus::vk
{
    DescriptorPoolRef DescriptorPool::CreateDescriptorPool(int numBuffers, int numSamplers, int numSets)
    {
        return std::make_shared<DescriptorPool>(numBuffers, numSamplers, numSets);
    }

    DescriptorPool::DescriptorPool(int numBuffers, int numSamplers, int numSets)
    {
        std::vector<VkDescriptorPoolSize> poolSizes;
        if (numBuffers > 0)
        {
            VkDescriptorPoolSize uniformPoolSize{};
            uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uniformPoolSize.descriptorCount = numBuffers;
            poolSizes.push_back(uniformPoolSize);
        }

        if (numSamplers > 0)
        {
            VkDescriptorPoolSize imageSamplerPoolSize{};
            imageSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            imageSamplerPoolSize.descriptorCount = numSamplers;
            poolSizes.push_back(imageSamplerPoolSize);
        }

        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = numSets;
        descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        CHECK_VK_RESULT(vkCreateDescriptorPool(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &descriptorPoolInfo, nullptr, &m_DescriptorPool));
    }

    DescriptorPool::~DescriptorPool()
    { 
        vkDestroyDescriptorPool(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), m_DescriptorPool, nullptr);
    }

    void DescriptorPool::AllocateDescriptorSet(DescriptorSet* descriptorSet, DescriptorSetLayout* layout)
    {
		VkDescriptorSetAllocateInfo allocateInfo = GetAllocationInfo(layout->GetVulkanDescriptorSetLayout());
		CHECK_VK_RESULT(vkAllocateDescriptorSets(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), &allocateInfo, &descriptorSet->GetVulkanDescriptorSet()));
    }

    void DescriptorPool::FreeDescriptorSet(DescriptorSet* descriptorSet)
    {
        vkFreeDescriptorSets(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), m_DescriptorPool, 1, &descriptorSet->GetVulkanDescriptorSet());
    }

    VkDescriptorSetAllocateInfo DescriptorPool::GetAllocationInfo(const VkDescriptorSetLayout& layout)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
	    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	    allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.pSetLayouts = &layout;
	    allocInfo.descriptorSetCount = 1;

	    return allocInfo;
    }
}