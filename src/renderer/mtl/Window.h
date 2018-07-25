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
        void Destroy() override;
        uint32_t GetWidth() override;
        uint32_t GetHeight() override;
        
        glm::vec2 GetMousePos() override;
        
        void* GetView() { return m_View; }
        
    private:
        
        void* m_Window;
        void* m_View;
    };
}
