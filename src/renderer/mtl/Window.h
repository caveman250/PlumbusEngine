#pragma once
#include "renderer/base/Window.h"

namespace mtl
{
    class Window : public base::Window
    {
    public:
        Window() {}
        
        ~Window() {}
        
        void Init(uint32_t width, uint32_t height) override;
        uint32_t GetWidth() override;
        uint32_t GetHeight() override;
        
        void* GetView() { return m_View; }
        
    private:
        
        void* m_Window;
        void* m_View;
    };
}
