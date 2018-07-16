#pragma once
#include "renderer/base/Renderer.h"
#include <functional>

namespace mtl
{
    class MetalRenderer : public base::Renderer
    {
    public:
        void Init() override;
        void Cleanup() override;
        
        void DrawFrame() override;
        bool WindowShouldClose() override;
        
        void AwaitIdle() override;
    };
}

