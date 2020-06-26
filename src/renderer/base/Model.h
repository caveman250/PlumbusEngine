#pragma once
#include "plumbus.h"
#include "renderer/base/Renderer.h"
#include "renderer/base/Texture.h"
#include "renderer/base/Material.h"
#include "components/ModelComponent.h"
#include "renderer/base/renderer_fwd.h"

struct ModelPart
{
    uint32_t m_VertexBase;
    uint32_t m_VertexCount;
    uint32_t m_IndexBase;
    uint32_t m_IndexCount;
};

struct Dimension
{
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);
    glm::vec3 size;
};

namespace plumbus::base
{
	class Model
	{
	public:
		Model();
		virtual ~Model() {}

		virtual void LoadModel(const std::string& fileName) = 0;
		virtual void Cleanup() = 0;
		virtual void UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo) = 0;
		virtual void Setup(Renderer* renderer) = 0;
		virtual void SetMaterial(plumbus::MaterialRef material) { m_Material = material; }
        
		Texture* GetColourMap();
		Texture* GetNormalMap();

    protected:
		Texture* m_ColourMap;
		Texture* m_NormalMap;
		plumbus::MaterialRef m_Material;

        void LoadFromFile(const std::string& fileName,
                          std::vector<VertexLayoutComponent> vertLayoutComponents,
                          std::vector<float>& vertexBuffer,
                          std::vector<uint32_t>& indexBuffer);
	};
}
