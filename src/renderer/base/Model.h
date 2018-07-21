#pragma once
#include <string>
#include "renderer/base/Renderer.h"
#include "renderer/base/Texture.h"
#include "components/ModelComponent.h"

enum VertexLayoutComponent
{
    VERTEX_COMPONENT_POSITION = 0x0,
    VERTEX_COMPONENT_NORMAL = 0x1,
    VERTEX_COMPONENT_COLOR = 0x2,
    VERTEX_COMPONENT_UV = 0x3,
    VERTEX_COMPONENT_TANGENT = 0x4,
    VERTEX_COMPONENT_BITANGENT = 0x5,
    VERTEX_COMPONENT_DUMMY_FLOAT = 0x6,
    VERTEX_COMPONENT_DUMMY_VEC4 = 0x7
};
struct VertexLayout
{
public:
    std::vector<VertexLayoutComponent> components;
    
    VertexLayout(std::vector<VertexLayoutComponent> components)
    {
        this->components = std::move(components);
    }
    
    uint32_t stride()
    {
        uint32_t res = 0;
        for (auto& component : components)
        {
            switch (component)
            {
                case VERTEX_COMPONENT_UV:
                    res += 2 * sizeof(float);
                    break;
                case VERTEX_COMPONENT_DUMMY_FLOAT:
                    res += sizeof(float);
                    break;
                case VERTEX_COMPONENT_DUMMY_VEC4:
                    res += 4 * sizeof(float);
                    break;
                default:
                    // All components except the ones listed above are made up of 3 floats
                    res += 3 * sizeof(float);
            }
        }
        return res;
    }
};

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

namespace base
{
	class Model
	{
	public:
		Model() {}
		~Model() {}

		virtual void LoadModel(const std::string& fileName) = 0;
		virtual void Cleanup() = 0;
		virtual void UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo) = 0;
		virtual void Setup(Renderer* renderer) = 0;

		base::Texture* m_ColourMap;
		base::Texture* m_NormalMap;
        
    protected:
        void LoadFromFile(const std::string& fileName,
                          std::vector<VertexLayoutComponent> vertLayoutComponents,
                          std::vector<float>& vertexBuffer,
                          std::vector<uint32_t>& indexBuffer);
	};
}
