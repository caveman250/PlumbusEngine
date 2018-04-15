#pragma once
#include "components/Component.h"
#include "glm/glm.hpp"


class PointLightComponent : public Component
{
public:
	PointLightComponent(glm::vec3 colour, float radius);
	~PointLightComponent() {}

	void OnUpdate(Scene* scene) override;

	glm::vec3 GetColour() { return m_Colour; }
	float GetRadius() { return m_Radius; }

	static const ComponentType GetType() { return Component::PointLightComponent; }

private:
	glm::vec3 m_Colour;
	float m_Radius;
};