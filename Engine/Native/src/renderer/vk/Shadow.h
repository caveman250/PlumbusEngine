#pragma once
#include "plumbus.h"
#include "components/LightComponent.h"
#include "Buffer.h"

namespace plumbus
{
    class Light;
}

namespace plumbus::vk
{
    class Shadow
    {
        public:
            Shadow(Light* light) : m_Light(light) {}
    		virtual ~Shadow();
            virtual void Init() = 0;
            
            FrameBufferRef GetFrameBuffer() const { return m_FrameBuffer; }
            VkSemaphore GetSemaphore() const { return m_Semaphore; }
            CommandBufferRef GetCommandBuffer() const { return m_CommandBuffer; }

    protected:
            FrameBufferRef m_FrameBuffer = VK_NULL_HANDLE;
            VkSemaphore m_Semaphore = VK_NULL_HANDLE; 
            CommandBufferRef m_CommandBuffer = VK_NULL_HANDLE;
            Light* m_Light;
    };
}
