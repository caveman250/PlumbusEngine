#pragma once
#include "GameComponent.h"
#include "glm/glm.hpp"

namespace plumbus
{
	class TranslationComponent : public GameComponent
	{
	public:
		TranslationComponent();

		void OnUpdate(Scene* scene) override;

		glm::vec3 GetTranslation();
		glm::vec3 GetRotation();
		glm::vec3 GetScale();

		void SetTranslation(glm::vec3 translation);
		void SetRotation(glm::vec3 rotation);
		void SetScale(glm::vec3 scale);

		void Translate(glm::vec3 translation);
		void Rotate(glm::vec3 rotation);
		void Scale(glm::vec3 scale);

		static const ComponentType GetType() { return GameComponent::TranslationComponent; }

	private:
		glm::vec3 m_Translation;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale = glm::vec3(1, 1, 1);
	};
}