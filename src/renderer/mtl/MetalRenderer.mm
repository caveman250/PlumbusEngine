#include "renderer/mtl/MetalRenderer.h"
#include "renderer/mtl/Window.h"
#include "renderer/mtl/Model.h"
#include "renderer/mtl/Texture.h"

#include "Application.h"
#import <Foundation/Foundation.h>

#import <Metal/Metal.h>
#import <Metal/MTLDevice.h>
#import <MetalKit/MetalKit.h>

#include "GameObject.h"
#include "components/TranslationComponent.h"
#include "components/ModelComponent.h"

#import "renderer/mtl/MetalView.h"

#include <stdint.h>

@interface RendererObjC : NSObject
{
    @public id<MTLDevice> m_Device;
    id<MTLCommandQueue> m_CommandQueue;
    id<MTLRenderPipelineState> m_RenderPipelineState;
    id<MTLDepthStencilState> m_DepthStencilState;
    dispatch_semaphore_t m_DisplaySemaphore;
}

-(void) Init;
-(void) DrawFrame:(nonnull MetalView*)view;
-(id<MTLDevice>) GetDevice;
-(id<MTLCommandQueue>) GetCommandQueue;

@end

@implementation RendererObjC

-(void)Init
{
    m_Device = MTLCreateSystemDefaultDevice();
    m_CommandQueue = [m_Device newCommandQueue];

    m_DisplaySemaphore = dispatch_semaphore_create(1);
    
    //Create Shaders
    id<MTLLibrary> defaultLibrary = [m_Device newDefaultLibrary];
    id<MTLFunction> vertexFunc = [defaultLibrary newFunctionWithName:@"basic_vertex"];
    id<MTLFunction> fragFunc = [defaultLibrary newFunctionWithName:@"basic_fragment"];

    MTLRenderPipelineDescriptor* renderPipelineDesc = [MTLRenderPipelineDescriptor new];
    [renderPipelineDesc setVertexFunction:vertexFunc];
    [renderPipelineDesc setFragmentFunction:fragFunc];
    [renderPipelineDesc.colorAttachments[0] setPixelFormat:MTLPixelFormatBGRA8Unorm];
    renderPipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    m_RenderPipelineState = [m_Device newRenderPipelineStateWithDescriptor:renderPipelineDesc error:nil];
    
    MTLDepthStencilDescriptor* depthStencilDescriptor = [MTLDepthStencilDescriptor new];
    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
    depthStencilDescriptor.depthWriteEnabled = YES;
    m_DepthStencilState = [m_Device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
    
}

-(void)DrawFrame:(nonnull MetalView*)view;
{
    dispatch_semaphore_wait(m_DisplaySemaphore, DISPATCH_TIME_FOREVER);
    
    id<MTLCommandBuffer> commandBuffer = [m_CommandQueue commandBuffer];
    
    MTLRenderPassDescriptor* renderPassDesc = [view m_CurrentRenderPassDescriptor];

    id<MTLRenderCommandEncoder> renderPass = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDesc];
    [renderPass setRenderPipelineState:m_RenderPipelineState];
    [renderPass setDepthStencilState:m_DepthStencilState];
    [renderPass setFrontFacingWinding:MTLWindingCounterClockwise];
    [renderPass setCullMode:MTLCullModeNone];

    for (GameObject* obj : Application::Get().GetScene()->GetObjects())
    {
        if (ModelComponent* component = obj->GetComponent<ModelComponent>())
        {
            mtl::Model* model = static_cast<mtl::Model*>(component->GetModel());
            
            [renderPass setVertexBuffer:(id<MTLBuffer>)model->GetVertexBuffer() offset:0 atIndex:0];
            [renderPass setVertexBuffer:(id<MTLBuffer>)model->GetUniformBuffer() offset:0 atIndex:1];
            
            mtl::Texture* diffuse = (mtl::Texture*)model->m_ColourMap;
            mtl::Texture* normal = (mtl::Texture*)model->m_NormalMap;
            
            [renderPass setFragmentTexture:(id<MTLTexture>)diffuse->GetTexture() atIndex:0];
            [renderPass setFragmentSamplerState:(id<MTLSamplerState>)diffuse->GetSamplerState() atIndex:0];
            [renderPass setFragmentTexture:(id<MTLTexture>)normal->GetTexture() atIndex:1];
            [renderPass setFragmentSamplerState:(id<MTLSamplerState>)normal->GetSamplerState() atIndex:1];
            
            [renderPass drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                    indexCount:[(id<MTLBuffer>)model->GetIndexBuffer() length] / sizeof(uint32_t)
                                    indexType:MTLIndexTypeUInt32
                                    indexBuffer:(id<MTLBuffer>)model->GetIndexBuffer()
                                    indexBufferOffset:0];
        }
    }
    
    [renderPass endEncoding];
    
    [commandBuffer presentDrawable:view.m_CurrentDrawable];
    
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer)
    {
        dispatch_semaphore_signal(m_DisplaySemaphore);
    }];
    
    [commandBuffer commit];
}

-(id<MTLDevice>) GetDevice
{
    return m_Device;
}

-(id<MTLCommandQueue>) GetCommandQueue
{
    return m_CommandQueue;
}

@end

namespace mtl
{
    void MetalRenderer::Init()
    {
        m_ObjcManager = [RendererObjC new];
        [(RendererObjC*)m_ObjcManager Init];
        
        m_Window = new mtl::Window();
        m_Window->Init(720, 480);
        
        GameObject* obj = new GameObject("Knight");
        Application::Get().GetScene()->AddGameObject(obj->
                                                     AddComponent<ModelComponent>(new ModelComponent("../models/armor.dae", "../textures/color_bc3_unorm.png", "../textures/normal_bc3_unorm.png"))->
                                                     AddComponent<TranslationComponent>(new TranslationComponent())
                                                     );
        
        GameObject* plane = new GameObject("Plane");
        Application::Get().GetScene()->AddGameObject(plane->
                                                     AddComponent<ModelComponent>(new ModelComponent("../models/plane.obj", "../textures/stonefloor01_color_bc3_unorm.png", "../textures/stonefloor01_normal_bc3_unorm.png"))->
                                                     AddComponent<TranslationComponent>(new TranslationComponent())
                                                     );

        plane->GetComponent<TranslationComponent>()->SetTranslation(glm::vec3(0, 2.3, 0));
        
        Application::Get().GetScene()->LoadModels();
        
        
        for (GameObject* obj : Application::Get().GetScene()->GetObjects())
        {
            if (ModelComponent* comp = obj->GetComponent<ModelComponent>())
                comp->GetModel()->Setup(this);
        }
    }
    
    void MetalRenderer::Cleanup()
    {
        //TODO
    }
    
    void MetalRenderer::DrawFrame()
    {
        MetalView* view = (MetalView*)static_cast<mtl::Window*>(m_Window)->GetView();
        [(RendererObjC*)m_ObjcManager DrawFrame:view];
    }
    
    bool MetalRenderer::WindowShouldClose()
    {
        //TODO
        return false;
    }
    
    void MetalRenderer::AwaitIdle()
    {
        //TODO
    }
    
    void* MetalRenderer::GetDevice()
    {
        return (void*)[(RendererObjC*)m_ObjcManager GetDevice];
        
    }
    
    void* MetalRenderer::GetCommandQueue()
    {
        return (void*)[(RendererObjC*)m_ObjcManager GetCommandQueue];
    }
}
