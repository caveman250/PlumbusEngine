#pragma once
#include <stdint.h>
#include <glm/glm.hpp>

namespace base
{
    class Window
    {
    public:        
        virtual void Init(uint32_t width, uint32_t height) = 0;
		virtual void Destroy() = 0;
        virtual uint32_t GetWidth() = 0;
        virtual uint32_t GetHeight() = 0;
        
        virtual glm::vec2 GetMousePos() = 0;
    };
}
