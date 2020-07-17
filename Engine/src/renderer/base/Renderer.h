#pragma once
#include "plumbus.h"

namespace plumbus::base
{
    class Window;
	class Renderer
	{
	public:
		virtual void Init() = 0;
		virtual void Cleanup() = 0;

		virtual void DrawFrame() = 0;
		virtual bool WindowShouldClose() = 0;

		virtual void AwaitIdle() = 0;

		virtual void OnModelAddedToScene() = 0;
		virtual void OnModelRemovedFromScene() = 0;
        
        Window* GetWindow() { return m_Window; };


        
    protected:
        Window* m_Window;
        
	};
}
