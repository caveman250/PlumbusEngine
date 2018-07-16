#include "Application.h"

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <set>
#include <algorithm>
#include "Helpers.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <cstring>

#include "GameObject.h"
#include "components/GameComponent.h"
#include "components/ModelComponent.h"
#include "components/TranslationComponent.h"
#include "Camera.h"
#include "components/PointLightComponent.h"

#include "renderer/base/Renderer.h"
#include "renderer/mtl/MetalRenderer.h"

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

@interface WindowViewController : NSViewController<MTKViewDelegate>
{
@public mtl::MetalRenderer* m_Renderer;
@public Application* m_App;
@public void (mtl::MetalRenderer::* m_Render)();
@public void (Application::* m_initScene)();
@public void (Application::* m_Update)();
@public NSTimer* m_Timer;
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

-(void)update
{
    (m_App->*m_Update)();
}

-(void)setupTimer
{
    [m_Timer invalidate];
    NSTimeInterval interval = 1.0 / 60.0;
    m_Timer = [NSTimer scheduledTimerWithTimeInterval:interval
                                               target:self
                                             selector:@selector(update)
                                             userInfo:nil
                                              repeats:YES];
}

@end

Application* Application::m_Instance = nullptr;

Application::Application() :
m_Scene(new Scene()),
m_lastUpdateTime(glfwGetTime())
{
    m_Renderer = new mtl::MetalRenderer();
}

void Application::Run()
{
    m_Renderer->Init();
    InitScene();
    
    NSRect frame = NSMakeRect(0, 0, 720, 480);
    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame
                                                   styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable)
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    window.title = [[NSProcessInfo processInfo] processName];
    WindowViewController* viewController = [WindowViewController new];
    viewController->m_Renderer = static_cast<mtl::MetalRenderer*>(m_Renderer);
    viewController->m_Render = &mtl::MetalRenderer::DrawFrame;
    viewController->m_App = &Application::Get();
    viewController->m_Update = &Application::MainLoop;
    
    MTKView* view = [[MTKView alloc] initWithFrame:frame];
    //view.device = (__bridge id<MTLDevice>)device.GetPtr();
    view.delegate = viewController;
    view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    
    [window.contentView addSubview:view];
    [window center];
    [window orderFrontRegardless];;
    
    //m_View = (__bridge void*)view;
    
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    
    NSMenu* menubar = [NSMenu new];
    NSMenuItem* appMenuItem = [NSMenuItem new];
    NSMenu* appMenu = [NSMenu new];
    NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(stop:) keyEquivalent:@"q"];
    [menubar addItem:appMenuItem];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
    [NSApp setMainMenu:menubar];
    
    [NSApp activateIgnoringOtherApps:YES];
    
    [viewController setupTimer];
    [NSApp run];
    
    m_Renderer->Cleanup();
}



void Application::InitScene()
{
    //probs have to do more here
    m_Scene->Init();
}


void Application::MainLoop()
{
    double currTime = glfwGetTime();
    m_DeltaTime = currTime - m_lastUpdateTime;
    m_lastUpdateTime = currTime;
    
    UpdateScene();
}

void Application::UpdateScene()
{
    m_Scene->OnUpdate();
}




