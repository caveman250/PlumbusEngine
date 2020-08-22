#pragma once
#include "plumbus.h"
#include "Shadow.h"
#include "Buffer.h"

namespace plumbus::vk
{
    class ShadowDirectional : public Shadow
    {
    public:
        static ShadowDirectionalRef CreateShadowDirectional(Light* light);

        ShadowDirectional(Light* light) : Shadow(light), m_ShadowDirectionalMaterialInstance(nullptr) {}
        virtual void Init() override;
        virtual void BuildCommandBuffer(uint32_t imageIndex) override;
        virtual void Render(uint32_t imageIndex, VkSemaphore waitSemaphore) override;
        virtual void CreateMaterialInstance() override;

        static void CleanupMaterial();

    private:
        static MaterialRef s_ShadowDirectionalMaterial;
        MaterialInstanceRef m_ShadowDirectionalMaterialInstance;

        struct UniformBufferObject
		{
			glm::mat4 m_Model;
			glm::mat4 m_View;
			glm::mat4 m_Proj;
		};

        UniformBufferObject m_UniformBufferObject;
        Buffer m_UniformBuffer;
    };
}