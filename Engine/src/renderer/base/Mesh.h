#pragma once
#include "plumbus.h"
#include "renderer/base/Renderer.h"
#include "renderer/base/Texture.h"
#include "renderer/base/Material.h"
#include "components/ModelComponent.h"

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
	class Mesh
	{
	public:
		Mesh();
		virtual ~Mesh() {}

		static  std::vector < base::Mesh*> LoadModel(const std::string& fileName, std::string defaultTexturePath, std::string defaultNormalPath);
		virtual void PostLoad() = 0;
		virtual void Cleanup() = 0;
		virtual void UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo) = 0;
		virtual void Setup() = 0;
		virtual void SetMaterial(plumbus::MaterialRef material) { m_Material = material; }
		std::vector<float>& GetVertexBuffer() { return m_VertexBuffer; }
		std::vector<uint32_t>& GetIndexBuffer() { return m_IndexBuffer; }
        
		Texture* GetColourMap();
		Texture* GetNormalMap();

    protected:
		Texture* m_ColourMap;
		Texture* m_NormalMap;
		plumbus::MaterialRef m_Material;

		std::vector<float> m_VertexBuffer;
		std::vector<uint32_t> m_IndexBuffer;

        static std::vector<base::Mesh*> LoadFromFile(const std::string& fileName,
                        std::vector<VertexLayoutComponent> vertLayoutComponents,
						std::string defaultDiffuseTexture,
						std::string defaultNormalTexture);
	};
}
