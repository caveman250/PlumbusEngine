#pragma once
#include <stdint.h>

namespace base
{
    class Window
    {
    public:        
        virtual void Init(uint32_t width, uint32_t height) = 0;
        virtual uint32_t GetWidth() = 0;
        virtual uint32_t GetHeight() = 0;
    };
}
