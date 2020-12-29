#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
    enum class DescriptorBindingUsage
    {
        VertexShader,
        FragmentShader
    };

    enum class DescriptorBindingType
    {
        UniformBuffer,
        ImageSampler
    };

    struct DescriptorBinding
    {
        DescriptorBindingType m_Type;
        DescriptorBindingUsage m_Usage;
        int m_Location;
        int m_Count;
        std::string m_Name;
    };
    
	class DescriptorSetLayout
	{
	public:
		static DescriptorSetLayoutRef CreateDescriptorSetLayout();

        DescriptorSetLayout();
        ~DescriptorSetLayout();

        void AddBinding(DescriptorBinding binding);
        void AddBinding(DescriptorBindingUsage usage, DescriptorBindingType type, int location, std::string name);
        void Build();

        const VkDescriptorSetLayout& GetVulkanDescriptorSetLayout() { return m_Layout; }
        std::vector<DescriptorBinding>& GetBindings() { return m_PendingBindings; }

    private:

        std::vector<DescriptorBinding> m_PendingBindings;
        VkDescriptorSetLayout m_Layout;
    };
}