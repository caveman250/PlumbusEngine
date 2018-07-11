#pragma once
#include "renderer/base/Renderer.h"

namespace mtl
{
    class MetalRenderer : public base::Renderer
    {
        void Init() override;
        void Cleanup() override;
        
        void DrawFrame() override;
        bool WindowShouldClose() override;
        
        void AwaitIdle() override;
    };
}

