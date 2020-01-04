#pragma once
#include "plumbus.h"
#include "renderer/vk/Buffer.h"
#include "renderer/vk/Texture.h"
#include "glm/glm.hpp"
#include "components/ModelComponent.h"
#include "renderer/base/Model.h"
#include "renderer/vk/Material.h"

namespace plumbus::vk
{
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
		void SetMaterial(MaterialRef material) override;

		void CreateUniformBuffer(vk::VulkanDevice* vulkanDevice);
		void CreateDescriptorSet(VkDescriptorSetAllocateInfo allocInfo);
		void SetupCommandBuffer(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout);

		void UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo) override;

		vk::Buffer& GetVertexBuffer();
		vk::Buffer& GetIndexBuffer();

		//todo there should really be a constructor for custom geometry, remove this once added.
		void SetIndexSize(uint32_t indexSize); 
private:
		uint32_t m_IndexSize;

		vk::Buffer m_VertexBuffer;
		vk::Buffer m_IndexBuffer;

		VkDescriptorSet m_DescriptorSet;

		vk::Buffer m_UniformBuffer;

		VertexLayout m_VertexLayout;
	};
}
