#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Component.h"
#include <string>
#include "glm/glm.hpp"

class Model;
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
	ModelComponent(std::string modelPath, std::string texturePath);
	~ModelComponent();
	Model* GetModel();
	void LoadModel();
	void OnUpdate(Scene* scene) override;
	void UpdateUniformBuffer(Scene* scene);
	void CreateUniformBuffer();
	void CreateDescriptorSet();

	VkDescriptorSet* GetDescriptorSet() { return &m_DescriptorSet; }

private:
	std::string m_ModelPath;
	std::string m_TexturePath;

	VkBuffer m_UniformBuffer;
	VkDeviceMemory m_UniformBufferMemory;

	VkDescriptorSet m_DescriptorSet;

	Model* m_Model;
};