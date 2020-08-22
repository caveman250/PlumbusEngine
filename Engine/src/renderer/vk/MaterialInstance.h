#pragma once

#include "plumbus.h"

namespace plumbus::vk
{
    class Buffer;
	class MaterialInstance
	{
	public:
        static MaterialInstanceRef CreateMaterialInstance(MaterialRef material);

		MaterialInstance(MaterialRef material);
		~MaterialInstance();

        void SetTextureUniform(std::string name, VkSampler sampler, VkImageView imageView);
		void SetBufferUniform(std::string name, Buffer* buffer);

        void Bind(CommandBufferRef commandBuffer);

	private:
		MaterialRef m_Material;
        DescriptorSetRef m_DescriptorSet;

        bool m_UniformsDirty;
	};
}