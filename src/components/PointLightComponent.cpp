#include "plumbus.h"
#include "PointLightComponent.h"
#include "glm/glm.hpp"

namespace plumbus
{
	PointLightComponent::PointLightComponent(glm::vec3 colour, float radius)
		: GameComponent()
	{
		m_Colour = colour;
		m_Radius = radius;
	}

	void PointLightComponent::OnUpdate(Scene* scene)
	{

	}
}
