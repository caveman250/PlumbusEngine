#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
	class DescriptorSetLayout
	{
	public:
        enum class BindingUsage
		{
			VertexShader,
			FragmentShader
		};

        enum class BindingType
		{
			UniformBuffer,
			ImageSampler
		};

        struct Binding
        {
            BindingType m_Type;
            BindingUsage m_Usage;
            int m_Location;
        };

		static DescriptorSetLayoutRef CreateDescriptorSetLayout();

        DescriptorSetLayout();
        ~DescriptorSetLayout();

        void AddBinding(Binding binding);
        void AddBinding(BindingUsage usage, BindingType type, int location);
        void Build();

        const VkDescriptorSetLayout& GetVulkanDescriptorSetLayout() { return m_Layout; }

    private:

        std::vector<Binding> m_PendingBindings;
        VkDescriptorSetLayout m_Layout;
    };
}