#include "renderer/mtl/MetalRenderer.h"
#include "renderer/mtl/Window.h"

#include "Application.h"
#import <Foundation/Foundation.h>

#import <Metal/Metal.h>
#import <Metal/MTLDevice.h>
#import <MetalKit/MetalKit.h>

@interface RendererObjC : NSObject
{
    @public id<MTLDevice> m_Device;
    id<MTLCommandQueue> m_CommandQueue;
    id<MTLRenderPipelineState> m_RenderPipelineState;
    id<MTLBuffer> m_VertexBuffer;
}

-(void) Init;
-(void) DrawFrame:(nonnull MTKView *)view;
-(id<MTLDevice>) GetDevice;

@end

@implementation RendererObjC

-(void)Init
{
    m_Device = MTLCreateSystemDefaultDevice();
    m_CommandQueue = [m_Device newCommandQueue];
    
    const float vertexData[] =
    {
        0.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
    };
    
    //Create Shaders
    id<MTLLibrary> defaultLibrary = [m_Device newDefaultLibrary];
    id<MTLFunction> vertexFunc = [defaultLibrary newFunctionWithName:@"basic_vertex"];
    id<MTLFunction> fragFunc = [defaultLibrary newFunctionWithName:@"basic_fragment"];
    
    m_VertexBuffer = [m_Device newBufferWithBytes:vertexData length:sizeof(vertexData) options:MTLResourceCPUCacheModeDefaultCache];
    
    MTLRenderPipelineDescriptor* renderPipelineDesc = [MTLRenderPipelineDescriptor new];
    [renderPipelineDesc setVertexFunction:vertexFunc];
    [renderPipelineDesc setFragmentFunction:fragFunc];
    [renderPipelineDesc.colorAttachments[0] setPixelFormat:MTLPixelFormatBGRA8Unorm];
    m_RenderPipelineState = [m_Device newRenderPipelineStateWithDescriptor:renderPipelineDesc error:nil];
}

-(void)DrawFrame:(nonnull MTKView *)view;
{
    id<MTLCommandBuffer> commandBuffer = [m_CommandQueue commandBuffer];
    
    MTLRenderPassDescriptor* renderPassDesc = [view currentRenderPassDescriptor];
    if (renderPassDesc)
    {
        id<MTLRenderCommandEncoder> renderCommandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDesc];
        [renderCommandEncoder setRenderPipelineState:m_RenderPipelineState];
        [renderCommandEncoder setVertexBuffer:m_VertexBuffer offset:0 atIndex:0];
        [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
        [renderCommandEncoder endEncoding];
        [commandBuffer presentDrawable:[view currentDrawable]];
    }
    
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
}

-(id<MTLDevice>) GetDevice
{
    return m_Device;
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
    }
    
    void MetalRenderer::Cleanup()
    {
        //TODO
    }
    
    void MetalRenderer::DrawFrame()
    {
        MTKView* view = (MTKView*)static_cast<mtl::Window*>(m_Window)->GetView();
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
}
