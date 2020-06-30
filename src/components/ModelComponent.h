#pragma once

#include "plumbus.h"

#include "GameComponent.h"
#include "renderer/base/renderer_fwd.h"

namespace plumbus::base
{
	class Mesh;
	class Material;
	class MaterialRef;
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
		ModelComponent(std::string modelPath, std::string texturePath, std::string normalPath, base::Material* material);
		~ModelComponent();
		std::vector<base::Mesh*> GetModels();
		void LoadModel();
		void SetMaterial(MaterialRef material);

		void OnUpdate(Scene* scene) override;
		void Cleanup();
		void UpdateUniformBuffer(Scene* scene);

		std::string GetModelPath() { return m_ModelPath; }
		std::string GetTexturePath() { return m_TexturePath; }
		std::string GetNormalPath() { return m_NormalPath; }

		static const ComponentType GetType() { return GameComponent::ModelComponent; }

	private:

		UniformBufferObject m_UniformBufferObject;

		std::vector<base::Mesh*> m_Models;
		MaterialRef m_Material;

		std::string m_ModelPath;
		std::string m_TexturePath;
		std::string m_NormalPath;
	};
}