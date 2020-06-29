#include "plumbus.h"

#include "components/ModelComponent.h"
#include "BaseApplication.h"
#include "TranslationComponent.h"
#include "Helpers.h"
#include "GameObject.h"
#include "renderer/base/Model.h"
#include "Scene.h"

#if METAL_RENDERER
#include "renderer/mtl/Model.h"
#elif VULKAN_RENDERER
#include "renderer/vk/Model.h"
#endif

namespace plumbus
{
	ModelComponent::ModelComponent(std::string modelPath, std::string texturePath, std::string normalPath)
		: GameComponent()
		, m_ModelPath(modelPath)
		, m_TexturePath(texturePath)
		, m_NormalPath(normalPath)
		, m_Model(nullptr)
		, m_Material(nullptr)
	{
		
	}

	ModelComponent::ModelComponent(std::string modelPath, std::string texturePath, std::string normalPath, base::Material* material)
		: GameComponent()
		, m_ModelPath(modelPath)
		, m_TexturePath(texturePath)
		, m_NormalPath(normalPath)
		, m_Model(nullptr)
		, m_Material(material)
	{
		
	}

	ModelComponent::~ModelComponent()
	{

	}

	base::Model* ModelComponent::GetModel()
	{
		return m_Model;
	}

	void ModelComponent::LoadModel()
	{
#if VULKAN_RENDERER
		m_Model = new vk::Model();
#elif METAL_RENDERER
		m_Model = new mtl::Model();
#endif
		m_Model->LoadModel(m_ModelPath);

		m_Model->GetColourMap()->LoadTexture(m_TexturePath);
		m_Model->GetNormalMap()->LoadTexture(m_NormalPath);

		if (m_Material)
		{
			m_Model->SetMaterial(m_Material);
		}
	}

	void ModelComponent::SetMaterial(MaterialRef material)
	{
		if (m_Material != material)
		{
			m_Material = material;
			if (m_Model)
			{
				m_Model->SetMaterial(material);
			}
		}
	}

	void ModelComponent::OnUpdate(Scene* scene)
	{
		UpdateUniformBuffer(scene);
	}

	void ModelComponent::Cleanup()
	{
		if (m_Model)
		{
			m_Model->Cleanup();
			delete m_Model;
			m_Model = nullptr;
		}
	}

	void ModelComponent::UpdateUniformBuffer(Scene* scene)
	{
		if (m_Model)
		{
			m_UniformBufferObject.m_Proj = scene->GetCamera()->GetProjectionMatrix();
			m_UniformBufferObject.m_View = scene->GetCamera()->GetViewMatrix();

			::plumbus::TranslationComponent* transComp = GetOwner()->GetComponent< ::plumbus::TranslationComponent>();
			glm::mat4 model = glm::identity<glm::mat4>();

			glm::vec3 translation = transComp->GetTranslation();
			glm::vec3 rotation = transComp->GetRotation();
			glm::vec3 scale = transComp->GetScale();

			model = glm::translate(model, translation);
			model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

			model = glm::scale(model, scale);

			m_UniformBufferObject.m_Model = model;

			m_Model->UpdateUniformBuffer(m_UniformBufferObject);
		}
	}
}