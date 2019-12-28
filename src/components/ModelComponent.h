#pragma once

#include "plumbus.h"

#include "GameComponent.h"
#include "renderer/vk/Buffer.h"

namespace plumbus::base
{
	class Model;
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
}