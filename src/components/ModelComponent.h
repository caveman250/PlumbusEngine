#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Component.h"
#include <string>
#include "glm/glm.hpp"
#include "vk/Buffer.h"
#include "vk/Model.h"

namespace vk
{
	class Model;
	class VulkanDevice;
}
class Scene;
class ModelComponent : public Component
{
	struct UniformBufferObject
	{
		glm::mat4 m_Model;
		glm::mat4 m_View;
		glm::mat4 m_Proj;
	};

public:

	ModelComponent(std::string modelPath, std::string texturePath, std::string normalPath);
	~ModelComponent();
	vk::Model* GetModel();
	void LoadModel(VkQueue queue, vk::VertexLayout layout);
	void OnUpdate(Scene* scene) override;
	void Cleanup(VkDevice device);


	void CreateUniformBuffer(vk::VulkanDevice* vulkanDevice);
	void CreateDescriptorSet(VkDescriptorSetAllocateInfo allocInfo);
	void SetupCommandBuffer(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout);

	static const ComponentType GetType() { return Component::ModelComponent; }

	std::string m_ModelPath;
	std::string m_TexturePath;
	std::string m_NormalPath;

private:
	void UpdateUniformBuffer(Scene* scene);

	vk::Buffer m_UniformBuffer;
	UniformBufferObject m_UniformBufferObject;

	VkDescriptorSet m_DescriptorSet;

	vk::Model* m_Model;
};