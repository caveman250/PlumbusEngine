#include "plumbus.h"
#include "LightComponent.h"
#include "glm/glm.hpp"

namespace plumbus
{
	LightComponent::LightComponent()
		: GameComponent()
	{
	}

	LightComponent::~LightComponent()
	{
		for (Light* light : m_Lights)
		{
			delete light;
			light = nullptr;
		}

		m_Lights.clear();
	}

	void LightComponent::AddPointLight(glm::vec3 colour, float radius)
	{
		m_Lights.push_back(new PointLight(colour, radius));
	}

	void LightComponent::AddDirectionalLight(glm::vec3 colour, glm::vec3 direction)
	{
		m_Lights.push_back(new DirectionalLight(colour, direction));
	}

	void LightComponent::OnUpdate(Scene* scene)
	{

	}
}
