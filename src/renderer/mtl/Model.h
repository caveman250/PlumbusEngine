#pragma once
#include <string>
#include "renderer/base/Model.h"
#include "renderer/mtl/MetalRenderer.h"

namespace mtl
{
    class Model : public base::Model
    {
    public:
        Model();
        ~Model();
        
        virtual void LoadModel(const std::string& filename) override;
        virtual void Cleanup() override;
        virtual void UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo) override;
        virtual void Setup(base::Renderer* renderer) override;
        
        void CreateUniformBuffer(void* device);
       // void CreateDescriptorSet(VkDescriptorSetAllocateInfo allocInfo);
        //void SetupCommandBuffer(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout);
        
        void* GetVertexBuffer();
        void* GetIndexBuffer();
        void* GetUniformBuffer();
        
        uint32_t m_IndexSize;
        
        void* m_ObjcManager;
    };
}
