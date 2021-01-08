#include "plumbus.h"

#include "TranslationComponent.h"

namespace plumbus::components
{

	TranslationComponent::TranslationComponent() 
		: GameComponent()
		, m_Translation(0)
		, m_Rotation(0)
		, m_Scale(1)
	{

	}

	glm::vec3 TranslationComponent::GetTranslation()
	{
		glm::vec3 trans = m_Translation;
		return m_Translation;
	}
	glm::vec3 TranslationComponent::GetRotation()
	{
		return m_Rotation;
	}
	glm::vec3 TranslationComponent::GetScale()
	{
		return m_Scale;
	}

	void TranslationComponent::OnUpdate(Scene* scene)
	{

	}

	void TranslationComponent::SetTranslation(glm::vec3 translation)
	{
		m_Translation = translation;
	}

	void TranslationComponent::SetRotation(glm::vec3 rotation)
	{
		m_Rotation = rotation;
	}

	void TranslationComponent::SetScale(glm::vec3 scale)
	{
		m_Scale = scale;
	}

	void TranslationComponent::Translate(glm::vec3 translation)
	{
		m_Translation += translation;
	}

	void TranslationComponent::Rotate(glm::vec3 rotation)
	{
		m_Rotation += rotation;
	}

	void TranslationComponent::Scale(glm::vec3 scale)
	{
		m_Scale += scale;
	}
}

#include "GameObject.h"
mono_vec3 TranslationComponent_GetTranslation(uint64_t obj)
{
    plumbus::components::TranslationComponent* objPtr = reinterpret_cast<plumbus::components::TranslationComponent*>(obj);
    return objPtr->GetTranslation();
}

mono_vec3 TranslationComponent_GetRotation(uint64_t obj)
{
    plumbus::components::TranslationComponent* objPtr = reinterpret_cast<plumbus::components::TranslationComponent*>(obj);
    return objPtr->GetRotation();
}

mono_vec3 TranslationComponent_GetScale(uint64_t obj)
{
    plumbus::components::TranslationComponent* objPtr = reinterpret_cast<plumbus::components::TranslationComponent*>(obj);
    return objPtr->GetScale();
}

void TranslationComponent_SetTranslation(uint64_t obj, mono_vec3 translation)
{
    plumbus::components::TranslationComponent* objPtr = reinterpret_cast<plumbus::components::TranslationComponent*>(obj);
    objPtr->SetTranslation({ translation.x, translation.y, translation.z });
}

void TranslationComponent_SetRotation(uint64_t obj, mono_vec3 rotation)
{
    plumbus::components::TranslationComponent* objPtr = reinterpret_cast<plumbus::components::TranslationComponent*>(obj);
    objPtr->SetRotation({ rotation.x, rotation.y, rotation.z });
}

void TranslationComponent_SetScale(uint64_t obj, mono_vec3 scale)
{
    plumbus::components::TranslationComponent* objPtr = reinterpret_cast<plumbus::components::TranslationComponent*>(obj);
    objPtr->SetScale({ scale.x, scale.y, scale.z });
}

void TranslationComponent_Translate(uint64_t obj, mono_vec3 translation)
{
    plumbus::components::TranslationComponent* objPtr = reinterpret_cast<plumbus::components::TranslationComponent*>(obj);
    objPtr->Translate({ translation.x, translation.y, translation.z });
}

void TranslationComponent_Rotate(uint64_t obj, mono_vec3 rotation)
{
    plumbus::components::TranslationComponent* objPtr = reinterpret_cast<plumbus::components::TranslationComponent*>(obj);
    objPtr->Rotate({ rotation.x, rotation.y, rotation.z });
}

void TranslationComponent_Scale(uint64_t obj, mono_vec3 scale)
{
    plumbus::components::TranslationComponent* objPtr = reinterpret_cast<plumbus::components::TranslationComponent*>(obj);
    objPtr->Scale({ scale.x, scale.y, scale.z });
}
