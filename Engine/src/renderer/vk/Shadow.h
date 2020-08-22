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
            virtual void Init() = 0;
            virtual void BuildCommandBuffer(uint32_t imageIndex) = 0;
            virtual void Render(uint32_t imageIndex, VkSemaphore waitSemaphore) = 0;
            virtual void CreateMaterialInstance() = 0;
            
            FrameBufferRef GetFrameBuffer(int index) { return m_FrameBuffers[index]; }
            VkSemaphore GetSemaphore(int index) { return m_Semaphores[index]; }
            CommandBufferRef GetCommandBuffer(int index) { return m_CommandBuffers[index]; }

    protected:
            std::vector<FrameBufferRef> m_FrameBuffers;
            std::vector<VkSemaphore> m_Semaphores; 
            std::vector<CommandBufferRef> m_CommandBuffers;
            Light* m_Light;
    };
}