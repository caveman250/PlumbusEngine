#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "GameComponent.h"
#include <string>
#include "glm/glm.hpp"
#include "renderer/vk/Buffer.h"

namespace base
{
	class Model;
}
class Scene;
class ModelComponent : public GameComponent
{
public:

	struct UniformBufferObject
	{
		glm::mat4 m_Model;
		glm::mat4 m_View;
		glm::mat4 m_Proj;
	};

	ModelComponent(std::string modelPath, std::string texturePath, std::string normalPath);
	~ModelComponent();
	base::Model* GetModel();
	void LoadModel();
	void OnUpdate(Scene* scene) override;
	void Cleanup();
	void UpdateUniformBuffer(Scene* scene);

	static const ComponentType GetType() { return GameComponent::ModelComponent; }

	std::string m_ModelPath;
	std::string m_TexturePath;
	std::string m_NormalPath;

private:
	
	UniformBufferObject m_UniformBufferObject;

	base::Model* m_Model;
};