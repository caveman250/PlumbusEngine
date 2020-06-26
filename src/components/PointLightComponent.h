#pragma once
#include "components/GameComponent.h"
#include "glm/glm.hpp"

namespace plumbus
{
	class PointLightComponent : public GameComponent
	{
	public:
		PointLightComponent(glm::vec3 colour, float radius);
		~PointLightComponent() {}

		void OnUpdate(Scene* scene) override;

		glm::vec3 GetColour() { return m_Colour; }
		float GetRadius() { return m_Radius; }

		void SetColour(glm::vec3 colour) { m_Colour = colour; }
		void SetRadius(float radius) { m_Radius = radius; }

		static const ComponentType GetType() { return GameComponent::PointLightComponent; }

	private:
		glm::vec3 m_Colour;
		float m_Radius;
	};
}