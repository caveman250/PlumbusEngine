#pragma once
#include "Component.h"
#include "glm/glm.hpp"


class TranslationComponent : public Component
{
public:
	TranslationComponent();

	void OnUpdate(Scene* scene) override;

	glm::vec3 GetTranslation() { return m_Translation; }
	glm::vec3 GetRotation() { return m_Rotation; }

	void SetTranslation(glm::vec3 translation);
	void SetRotation(glm::vec3 rotation);

	void Translate(glm::vec3 translation);
	void Rotate(glm::vec3 rotation);

	static const ComponentType GetType() { return Component::TranslationComponent; }

private:
	glm::vec3 m_Translation;
	glm::vec3 m_Rotation;
};