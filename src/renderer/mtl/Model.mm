#include "renderer/mtl/Model.h"
#include "renderer/mtl/Texture.h"
#include "Application.h"

#import "Metal/Metal.h"

@interface ModelObjc : NSObject
{
    @public id<MTLBuffer> m_VertexBuffer;
    @public id<MTLBuffer> m_IndexBuffer;
    @public id<MTLBuffer> m_UniformBuffer;
}

-(void)CreateVertexBuffer:(id<MTLDevice>) device withBytes:(void*)bytes withSize:(uint32_t)size;
-(void)CreateIndexBuffer:(id<MTLDevice>) device withBytes:(void*)bytes withSize:(uint32_t)size;
-(void)CreateUniformBuffer:(id<MTLDevice>) device withSize:(uint32_t)size;

@end

@implementation ModelObjc

-(void)CreateVertexBuffer:(id<MTLDevice>) device withBytes:(void*)bytes withSize:(uint32_t)size
{
    m_VertexBuffer = [device newBufferWithBytes:bytes length:size options:MTLResourceCPUCacheModeDefaultCache];
}
-(void)CreateIndexBuffer:(id<MTLDevice>) device withBytes:(void*)bytes withSize:(uint32_t)size
{
    m_IndexBuffer = [device newBufferWithBytes:bytes length:size options:MTLResourceCPUCacheModeDefaultCache];
}

-(void)CreateUniformBuffer:(id<MTLDevice>) device withSize:(uint32_t)size
{
    m_UniformBuffer = [device newBufferWithLength:size options:MTLResourceCPUCacheModeDefaultCache];
}

@end

namespace mtl
{
    Model::Model()
    {
        m_ObjcManager = [ModelObjc new];
        m_ColourMap = new mtl::Texture();
        m_NormalMap = new mtl::Texture();
    }
    
    Model::~Model(){}
    
    void Model::LoadModel(const std::string& fileName)
    {
        std::vector<VertexLayoutComponent> vertLayoutComponents;
        vertLayoutComponents.push_back(VERTEX_COMPONENT_POSITION);
        vertLayoutComponents.push_back(VERTEX_COMPONENT_DUMMY_FLOAT); //metal needs a vec 4 for position
        vertLayoutComponents.push_back(VERTEX_COMPONENT_UV);
        vertLayoutComponents.push_back(VERTEX_COMPONENT_DUMMY_FLOAT); //padding
        vertLayoutComponents.push_back(VERTEX_COMPONENT_DUMMY_FLOAT); //padding
        //ertLayoutComponents.push_back(VERTEX_COMPONENT_COLOR);
        //ertLayoutComponents.push_back(VERTEX_COMPONENT_NORMAL);
        //ertLayoutComponents.push_back(VERTEX_COMPONENT_TANGENT);
        
        std::vector<float> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
        
        LoadFromFile(fileName, vertLayoutComponents, vertexBuffer, indexBuffer);
        
        mtl::MetalRenderer* renderer = static_cast<mtl::MetalRenderer*>(Application::Get().GetRenderer());
        id<MTLDevice> device = (id<MTLDevice>)renderer->GetDevice();
        [(ModelObjc*)m_ObjcManager CreateVertexBuffer:device withBytes:vertexBuffer.data() withSize:vertexBuffer.size() * sizeof(float)];
        [(ModelObjc*)m_ObjcManager CreateIndexBuffer:device withBytes:indexBuffer.data() withSize:indexBuffer.size() * sizeof(uint32_t)];
    }
    
    void Model::Cleanup()
    {
        
    }
    
    void Model::UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo)
    {
        id<MTLBuffer> buffer = (id<MTLBuffer>)GetUniformBuffer();
        memcpy(buffer.contents, &ubo, sizeof(ModelComponent::UniformBufferObject));
    }
    
    void Model::Setup(base::Renderer* renderer)
    {
        CreateUniformBuffer(static_cast<MetalRenderer*>(renderer)->GetDevice());
    }
    
    void Model::CreateUniformBuffer(void* device)
    {
        uint32_t size = sizeof(ModelComponent::UniformBufferObject);
        
        [(ModelObjc*)m_ObjcManager CreateUniformBuffer:(id<MTLDevice>)device withSize:size];
    }
    
    void* Model::GetVertexBuffer()
    {
        return ((ModelObjc*)m_ObjcManager)->m_VertexBuffer;
    }
    void* Model::GetIndexBuffer()
    {
        return ((ModelObjc*)m_ObjcManager)->m_IndexBuffer;
    }
    
    void* Model::GetUniformBuffer()
    {
        return ((ModelObjc*)m_ObjcManager)->m_UniformBuffer;
    }
}
