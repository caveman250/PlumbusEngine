#include "PointLightComponent.h"
#include "glm/glm.hpp"

PointLightComponent::PointLightComponent(glm::vec3 colour, float radius)
	: Component()
{
	m_Colour = colour;
	m_Radius = radius;
}

void PointLightComponent::OnUpdate(Scene* scene)
{

}
