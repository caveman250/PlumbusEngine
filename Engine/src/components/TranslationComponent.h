#pragma once
#include "GameComponent.h"
#include "TranslationComponent.h"
#include "glm/glm.hpp"

namespace plumbus::components
{
	class TranslationComponent : public GameComponent
	{
	public:
		TranslationComponent();

		void Init() override {}
		void PostInit() override {}
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

#include "mono_impl/mono_fwd.h"

MONO_EXPORT(mono_vec3) GetTranslation(uint64_t obj);
MONO_EXPORT(mono_vec3) GetRotation(uint64_t obj);
MONO_EXPORT(mono_vec3) GetScale(uint64_t obj);
MONO_EXPORT(void) SetTranslation(uint64_t obj, mono_vec3 translation);
MONO_EXPORT(void) SetRotation(uint64_t obj, mono_vec3 rotation);
MONO_EXPORT(void) SetScale(uint64_t obj, mono_vec3 scale);
MONO_EXPORT(void) Translate(uint64_t obj, mono_vec3 translation);
MONO_EXPORT(void) Rotate(uint64_t obj, mono_vec3 rotation);
MONO_EXPORT(void) Scale(uint64_t obj, mono_vec3 scale);