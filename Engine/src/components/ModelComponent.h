#pragma once

#include "plumbus.h"

#include "GameComponent.h"

namespace plumbus::vk
{
	class Mesh;
	class Material;
}
namespace plumbus
{
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
		ModelComponent(std::string modelPath, std::string texturePath, std::string normalPath, vk::MaterialRef material);
		~ModelComponent();
		std::vector<vk::Mesh*> GetModels();
		void LoadModel();
		void SetMaterial(vk::MaterialRef material);

		void OnUpdate(Scene* scene) override;
		void Cleanup();
		void UpdateModelMatrix();
		void UpdateUniformBuffer(Scene* scene);

		std::string GetModelPath() { return m_ModelPath; }
		std::string GetTexturePath() { return m_TexturePath; }
		std::string GetNormalPath() { return m_NormalPath; }

		static const ComponentType GetType() { return GameComponent::ModelComponent; }

		glm::mat4 GetModelMatrix();

	private:

		UniformBufferObject m_UniformBufferObject;

		std::vector<vk::Mesh*> m_Models;
		vk::MaterialRef m_Material;

		std::string m_ModelPath;
		std::string m_TexturePath;
		std::string m_NormalPath;

		//dont update model matrix if we don't have to.
		glm::vec3 m_CachedPos;
		glm::vec3 m_CachedRotation;
		glm::vec3 m_CachedScale;

		glm::mat4 m_ModelMatrix;
	};
}