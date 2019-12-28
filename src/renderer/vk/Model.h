#pragma once
#include "plumbus.h"
#include "renderer/vk/Buffer.h"
#include "renderer/vk/Texture.h"
#include "glm/glm.hpp"
#include "components/ModelComponent.h"
#include "renderer/base/Model.h"

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
