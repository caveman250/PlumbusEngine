#pragma once

#include "plumbus.h"
#include "DescriptorSet.h"

namespace plumbus::vk
{
    class Buffer;
	class MaterialInstance
	{
	public:
        static MaterialInstanceRef CreateMaterialInstance(MaterialRef material);

		MaterialInstance(MaterialRef material);
		~MaterialInstance();

        void SetTextureUniform(std::string name, std::vector<DescriptorSet::TextureUniform> textureUniforms, bool isDepth);
		void SetBufferUniform(std::string name, Buffer* buffer);

        void Bind(CommandBufferRef commandBuffer);

		MaterialRef GetMaterial() { return m_Material; }

	private:
		MaterialRef m_Material;
        DescriptorSetRef m_DescriptorSet;

        bool m_UniformsDirty;
	};
}