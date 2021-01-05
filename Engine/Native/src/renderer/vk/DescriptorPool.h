#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
	class DescriptorPool
	{
	public:
		static DescriptorPoolRef CreateDescriptorPool(int numBuffers, int numSamplers, int numSets);

        DescriptorPool(int numBuffers, int numSamplers, int numSets);
        ~DescriptorPool();

        void AllocateDescriptorSet(DescriptorSet* descriptorSet, DescriptorSetLayout* layout);
        void FreeDescriptorSet(DescriptorSet* descriptorSet);

    private:
        VkDescriptorSetAllocateInfo GetAllocationInfo(const VkDescriptorSetLayout& layout);

        VkDescriptorPool m_DescriptorPool;
    };
}