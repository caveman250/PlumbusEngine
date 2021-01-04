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

