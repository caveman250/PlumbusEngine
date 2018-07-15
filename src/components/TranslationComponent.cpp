#include "TranslationComponent.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

TranslationComponent::TranslationComponent() :
	GameComponent()
{

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

