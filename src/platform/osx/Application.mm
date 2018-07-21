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
#include "renderer/mtl/Window.h"
#import "renderer/mtl/MetalView.h"

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

@interface ApplicationObjc : NSObject
{
    NSTimer* m_Timer;
    @public Application* m_App;
    @public void (Application::* m_UpdateFunc)();
}

-(void)StartFrameTimer;
-(void)Update;

@end

@implementation ApplicationObjc
-(void)StartFrameTimer
{
    [m_Timer invalidate];
    NSTimeInterval interval = 1.0 / 60.0;
    m_Timer = [NSTimer scheduledTimerWithTimeInterval:interval
                                               target:self
                                             selector:@selector(Update)
                                             userInfo:nil
                                              repeats:YES];
}

-(void)Update
{
    (m_App->*m_UpdateFunc)();
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
    m_ObjcManager = [ApplicationObjc new];
    ((ApplicationObjc*)m_ObjcManager)->m_App = this;
    ((ApplicationObjc*)m_ObjcManager)->m_UpdateFunc = &Application::MainLoop;
    [(ApplicationObjc*)m_ObjcManager StartFrameTimer];
    
    mtl::Window* window = static_cast<mtl::Window*>(m_Renderer->GetWindow());
    [(MetalView*)window->GetView() StartDrawing];

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




