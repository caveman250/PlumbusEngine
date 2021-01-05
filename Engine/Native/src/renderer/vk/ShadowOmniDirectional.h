#pragma once

#include "Shadow.h"
#include "Texture.h"
namespace plumbus::vk
{
    class ShadowOmniDirectional : public Shadow
    {
    public:
        static ShadowOmniDirectionalRef CreateShadowOmniDirectional(Light* light);

        ShadowOmniDirectional(Light* light)
        : Shadow(light)
                {
                }

        virtual ~ShadowOmniDirectional();

        virtual void Init() override;

        int GetRenderPassCount() { return 6; }
        void UpdateUniformBuffer();
        void BuildCommandBuffer(int index);
        void Render(VkSemaphore waitSemaphore);
        const Texture& GetCubeMap() const { return m_CubeMapTexture; }

    private:
        void SetupCubeMap();

        static MaterialRef s_ShadowOmniDirectionalMaterial;
        MaterialInstanceRef m_ShadowOmniDirectionalMaterialInstance;

        struct UniformBufferObject
        {
            glm::mat4 m_Proj;
            glm::vec4 m_LightPos;
        };

        struct PushContants
        {
            glm::mat4 view;
            glm::mat4 model;
        };

        UniformBufferObject m_UniformBufferObject;
        Buffer m_UniformBuffer;
        Texture m_CubeMapTexture;
    };
}
