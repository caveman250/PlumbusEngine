#pragma once
#include <string>
#include <vector>
#include "renderer/vk/Buffer.h"
#include "renderer/vk/Texture.h"
#include "glm/glm.hpp"
#include "ModelComponent.h"
#include "renderer/base/Model.h"

namespace vk
{
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
		/** @brief Components used to generate vertices from */
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

	class Scene;
	class VulkanDevice;
	class Model : public base::Model
	{
	public:
		Model();
		~Model();

		void LoadModel(const std::string& filename) override;
		void Cleanup() override;

		void Setup(base::Renderer* renderer) override;

		void CreateUniformBuffer(vk::VulkanDevice* vulkanDevice);
		void CreateDescriptorSet(VkDescriptorSetAllocateInfo allocInfo);
		void SetupCommandBuffer(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout);

		void UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo) override;

		uint32_t m_IndexSize;

		vk::Buffer m_VertexBuffer;
		vk::Buffer m_IndexBuffer;

		VkDescriptorSet m_DescriptorSet;

		vk::Buffer m_UniformBuffer;
	};
}
