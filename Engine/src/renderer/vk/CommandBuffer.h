#include "plumbus.h"
#include "Buffer.h"
#include "FrameBuffer.h"

namespace plumbus::vk
{
    class CommandBuffer
    {
        public:
            static CommandBufferRef CreateCommandBuffer();

            CommandBuffer(VkCommandBuffer cmdBuffer);
            ~CommandBuffer();

            VkCommandBuffer& GetVulkanCommandBuffer() { return m_CommandBuffer; }

            void BeginRecording() const;
            void BeginRenderPass() const;
            void EndRecording() const;
            void EndRenderPass() const;
            void RecordDraw(const uint32_t indexCount) const;

            void SetViewport(const float width, const float height, const float minDepth, const float maxDepth) const;
            void SetScissor(const uint32_t width, const uint32_t height, const int32_t minDepth, const int32_t maxDepth) const;
            void BindPipeline(const VkPipeline piepline) const;
            void BindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet descriptorSet) const;
            void BindVertexBuffer(const vk::Buffer& buffer) const;
            void BindIndexBuffer(const vk::Buffer& buffer) const;

            void SetFrameBuffer(FrameBufferRef frameBuffer) { m_FrameBuffer = frameBuffer; }
            
            void Cleanup();
        private:
            VkCommandBuffer m_CommandBuffer;
            FrameBufferRef m_FrameBuffer;
    };
}