#include "MaterialInstance.h"
#include "DescriptorSet.h"
#include "VulkanRenderer.h"
#include "CommandBuffer.h"

namespace plumbus::vk
{
    MaterialInstanceRef MaterialInstance::CreateMaterialInstance(MaterialRef material) 
    {
        return std::make_shared<MaterialInstance>(material);
    }
    
    MaterialInstance::MaterialInstance(MaterialRef material) 
        : m_UniformsDirty(true)
    {
        m_Material = material;
        m_DescriptorSet = DescriptorSet::CreateDescriptorSet(VulkanRenderer::Get()->GetDescriptorPool(), material->GetLayout());
    }
    
    MaterialInstance::~MaterialInstance() 
    {
        m_Material.reset();
        m_DescriptorSet.reset();
    }

    void MaterialInstance::SetTextureUniform(std::string name, std::vector<DescriptorSet::TextureUniform> textureUniforms, bool isDepth)
	{
        m_DescriptorSet->SetTextureUniform(name, textureUniforms, isDepth);
        m_UniformsDirty = true;
	}
	
	void MaterialInstance::SetBufferUniform(std::string name, Buffer* buffer) 
	{
        m_DescriptorSet->SetBufferUniform(name, buffer);
        m_UniformsDirty = true;
	}
    
    void MaterialInstance::Bind(CommandBufferRef commandBuffer) 
    {
        if (VulkanRenderer::Get()->GetBoundMaterialInstance() != this)
        {
            if (m_UniformsDirty)
            {
                m_DescriptorSet->Build();
                m_UniformsDirty = false;
            }

            commandBuffer->BindPipeline(m_Material->GetPipeline());
            commandBuffer->BindDescriptorSet(m_Material->GetPipelineLayout(), m_DescriptorSet);
            VulkanRenderer::Get()->SetBoundMaterial(this);
        }
    }


}