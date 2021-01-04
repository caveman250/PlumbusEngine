#include "plumbus.h"

#include "components/ModelComponent.h"
#include "BaseApplication.h"
#include "TranslationComponent.h"
#include "Helpers.h"
#include "GameObject.h"
#include "Scene.h"
#include "renderer/vk/Mesh.h"

namespace plumbus::components
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

	ModelComponent::ModelComponent(std::string modelPath, std::string texturePath, std::string normalPath, vk::MaterialRef material)
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

	std::vector<vk::Mesh*> ModelComponent::GetModels()
	{
		return m_Models;
	}

	void ModelComponent::LoadModel()
	{
		m_Models = vk::Mesh::LoadModel(m_ModelPath, m_TexturePath, m_NormalPath);

		if (m_Material)
		{
			for (vk::Mesh* model : m_Models)
			{
				model->SetMaterial(m_Material);
				model->Setup();
			}
		}
	}

	void ModelComponent::SetMaterial(vk::MaterialRef material)
	{
		if (m_Material != material)
		{
			m_Material = material;
			for (vk::Mesh* model : m_Models)
			{
				model->SetMaterial(material);
			}
		}
	}

	void ModelComponent::OnUpdate(Scene* scene)
	{
	    UpdateModelMatrix();
        UpdateUniformBuffer(scene);
	}

	void ModelComponent::Cleanup()
	{
		for (vk::Mesh* model : m_Models)
		{
			model->Cleanup();
			delete model;
			model = nullptr;
		}
	}

	void ModelComponent::UpdateUniformBuffer(Scene* scene)
	{
		for (vk::Mesh* model : m_Models)
		{
			m_UniformBufferObject.m_Proj = scene->GetCamera()->GetProjectionMatrix();
			m_UniformBufferObject.m_View = scene->GetCamera()->GetViewMatrix();
			m_UniformBufferObject.m_Model = GetModelMatrix();

			model->UpdateUniformBuffer(m_UniformBufferObject);
		}
	}
	
	glm::mat4 ModelComponent::GetModelMatrix() 
	{
        return m_ModelMatrix;
	}

    void ModelComponent::UpdateModelMatrix()
    {
        components::TranslationComponent* transComp = GetOwner()->GetComponent<components::TranslationComponent>();
        glm::vec3 translation = transComp->GetTranslation();
        glm::vec3 rotation = transComp->GetRotation();
        glm::vec3 scale = transComp->GetScale();

        if(translation != m_CachedPos ||
           rotation != m_CachedRotation ||
           scale != m_CachedScale)
        {


            m_CachedPos = translation;
            m_CachedRotation = rotation;
            m_CachedScale = scale;

            m_ModelMatrix = glm::identity<glm::mat4>();
            m_ModelMatrix = glm::translate(m_ModelMatrix, translation);
            m_ModelMatrix = glm::rotate(m_ModelMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            m_ModelMatrix = glm::rotate(m_ModelMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            m_ModelMatrix = glm::rotate(m_ModelMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

            m_ModelMatrix = glm::scale(m_ModelMatrix, scale);
        }
    }
}