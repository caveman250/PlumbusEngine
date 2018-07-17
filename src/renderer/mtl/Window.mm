#include "renderer/mtl/Window.h"
#include "Application.h"
#include "renderer/base/Renderer.h"
#include "renderer/mtl/MetalRenderer.h"

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

@interface WindowViewController : NSViewController<MTKViewDelegate>
{
@public mtl::MetalRenderer* m_Renderer;
@public void (mtl::MetalRenderer::* m_Render)();
}

@end

@implementation WindowViewController
-(void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    
}

-(void)drawInMTKView:(nonnull MTKView *)view
{
    (m_Renderer->*m_Render)();
}

@end

namespace mtl
{
    void Window::Init(uint32_t width, uint32_t height)
    {
        MetalRenderer* renderer = static_cast<MetalRenderer*>(Application::Get().GetRenderer());
        
        NSRect frame = NSMakeRect(0, 0, 720, 480);
        m_Window = [[NSWindow alloc] initWithContentRect:frame
                                                       styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable)
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        ((NSWindow*)m_Window).title = [[NSProcessInfo processInfo] processName];
        
        WindowViewController* viewController = [WindowViewController new];
        viewController->m_Renderer = renderer;
        viewController->m_Render = &mtl::MetalRenderer::DrawFrame;
        
        MTKView* view = [[MTKView alloc] initWithFrame:frame];
        view.device = (id<MTLDevice>)renderer->GetDevice();
        view.delegate = viewController;
        view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        
        m_View = view;
        
        [((NSWindow*)m_Window).contentView addSubview:view];
        [((NSWindow*)m_Window) center];
        [((NSWindow*)m_Window) orderFrontRegardless];;
    }
    
    uint32_t Window::GetWidth()
    {
        return [(NSWindow*)m_Window frame].size.width;
    }
    
    uint32_t Window::GetHeight()
    {
        return [(NSWindow*)m_Window frame].size.height;
    }
}
