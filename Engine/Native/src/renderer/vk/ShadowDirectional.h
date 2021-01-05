#pragma once
#include "plumbus.h"
#include "Shadow.h"
#include "Buffer.h"
namespace plumbus::components
{
    class ModelComponent;
}

namespace plumbus::vk
{
    class ShadowDirectional : public Shadow
    {
    public:
        static ShadowDirectionalRef CreateShadowDirectional(Light* light);

        ShadowDirectional(Light* light)
			: Shadow(light)
		{
		}

    	virtual ~ShadowDirectional();

        virtual void Init() override;
        virtual void BuildCommandBuffer();
        virtual void Render(VkSemaphore waitSemaphore);

    private:
        static MaterialRef s_ShadowDirectionalMaterial;
        std::unordered_map<components::ModelComponent*, MaterialInstanceRef> m_ShadowDirectionalMaterialInstances;

        struct UniformBufferObject
		{
			glm::mat4 m_Model;
			glm::mat4 m_View;
			glm::mat4 m_Proj;
		};

        std::unordered_map<components::ModelComponent*, UniformBufferObject> m_UniformBufferObjects;
        std::unordered_map<components::ModelComponent*, Buffer> m_UniformBuffers;
    };
}
