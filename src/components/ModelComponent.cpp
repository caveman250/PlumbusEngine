#include "plumbus.h"

#include "components/ModelComponent.h"
#include "BaseApplication.h"
#include "TranslationComponent.h"
#include "Helpers.h"
#include "GameObject.h"
#include "renderer/base/Mesh.h"
#include "Scene.h"

#if METAL_RENDERER
#include "renderer/mtl/Mesh.h"
#elif VULKAN_RENDERER
#include "renderer/vk/Mesh.h"
#endif

namespace plumbus
{
	ModelComponent::ModelComponent(std::string modelPath, std::string texturePath, std::string normalPath)
		: GameComponent()
		, m_ModelPath(modelPath)
		, m_TexturePath(texturePath)
		, m_NormalPath(normalPath)
		, m_Models()
		, m_Material(nullptr)
	{
		
	}

	ModelComponent::ModelComponent(std::string modelPath, std::string texturePath, std::string normalPath, base::Material* material)
		: GameComponent()
		, m_ModelPath(modelPath)
		, m_TexturePath(texturePath)
		, m_NormalPath(normalPath)
		, m_Models()
		, m_Material(material)
	{
		
	}

	ModelComponent::~ModelComponent()
	{

	}

	std::vector<base::Mesh*> ModelComponent::GetModels()
	{
		return m_Models;
	}

	void ModelComponent::LoadModel()
	{
		m_Models = base::Mesh::LoadModel(m_ModelPath, m_TexturePath, m_NormalPath);

		if (m_Material)
		{
			for (base::Mesh* model : m_Models)
			{
				model->SetMaterial(m_Material);
			}
		}
	}

	void ModelComponent::SetMaterial(MaterialRef material)
	{
		if (m_Material != material)
		{
			m_Material = material;
			for (base::Mesh* model : m_Models)
			{
				model->SetMaterial(material);
			}
		}
	}

	void ModelComponent::OnUpdate(Scene* scene)
	{
		UpdateUniformBuffer(scene);
	}

	void ModelComponent::Cleanup()
	{
		for (base::Mesh* model : m_Models)
		{
			model->Cleanup();
			delete model;
			model = nullptr;
		}
	}

	void ModelComponent::UpdateUniformBuffer(Scene* scene)
	{
		for (base::Mesh* model : m_Models)
		{
			m_UniformBufferObject.m_Proj = scene->GetCamera()->GetProjectionMatrix();
			m_UniformBufferObject.m_View = scene->GetCamera()->GetViewMatrix();

			::plumbus::TranslationComponent* transComp = GetOwner()->GetComponent< ::plumbus::TranslationComponent>();
			glm::mat4 modelMat = glm::identity<glm::mat4>();

			glm::vec3 translation = transComp->GetTranslation();
			glm::vec3 rotation = transComp->GetRotation();
			glm::vec3 scale = transComp->GetScale();

			modelMat = glm::translate(modelMat, translation);
			modelMat = glm::rotate(modelMat, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			modelMat = glm::rotate(modelMat, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMat = glm::rotate(modelMat, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

			modelMat = glm::scale(modelMat, scale);

			m_UniformBufferObject.m_Model = modelMat;

			model->UpdateUniformBuffer(m_UniformBufferObject);
		}
	}
}