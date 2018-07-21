#include "ModelComponent.h"
#include "Application.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "TranslationComponent.h"
#include "Helpers.h"
#include "GameObject.h"
#include <cstring>
#include "renderer/base/Model.h"

#if METAL_RENDERER
#include "renderer/mtl/Model.h"
#elif VULKAN_RENDERER
#include "renderer/vk/Model.h"
#endif

ModelComponent::ModelComponent(std::string modelPath, std::string texturePath, std::string normalPath)
	: GameComponent()
{
	m_ModelPath = modelPath;
	m_TexturePath = texturePath;
	m_NormalPath = normalPath;
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
	//m_Model->m_ColourMap->LoadTexture(m_TexturePath);
	//m_Model->m_NormalMap->LoadTexture(m_NormalPath);
}

void ModelComponent::OnUpdate(Scene* scene)
{
	UpdateUniformBuffer(scene);
}

void ModelComponent::Cleanup()
{
	m_Model->Cleanup();
}

void ModelComponent::UpdateUniformBuffer(Scene* scene)
{
	m_UniformBufferObject.m_Proj = scene->GetCamera()->GetProjectionMatrix();
	m_UniformBufferObject.m_View = scene->GetCamera()->GetViewMatrix();

	::TranslationComponent* transComp = GetOwner()->GetComponent< ::TranslationComponent>();
	glm::mat4 model;
	model = glm::translate(model, transComp->GetTranslation());
	model = glm::rotate(model, transComp->GetRotation().x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, transComp->GetRotation().y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, transComp->GetRotation().z, glm::vec3(0.0f, 0.0f, 1.0f));

	model = glm::scale(model, transComp->GetScale());

	m_UniformBufferObject.m_Model = model;

	m_Model->UpdateUniformBuffer(m_UniformBufferObject);
}
