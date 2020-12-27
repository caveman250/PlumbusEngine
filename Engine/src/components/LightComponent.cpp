#include "plumbus.h"
#include "LightComponent.h"
#include "glm/glm.hpp"
#include "renderer/vk/ShadowDirectional.h"
#include "renderer/vk/ShadowManager.h"

namespace plumbus
{
	void DirectionalLight::AddShadow() 
	{
		m_Shadow = std::static_pointer_cast<vk::Shadow>(vk::ShadowDirectional::CreateShadowDirectional(this));
		vk::ShadowManager::Get()->RegisterShadow(m_Shadow.get());
	}
	
	glm::mat4 DirectionalLight::GetMVP() 
	{
		glm::mat4 proj = glm::ortho<float>(-15,15,-15,15,-15,50);
        glm::mat4 view = glm::lookAt(GetDirection(), glm::vec3(0,0,0), glm::vec3(0, 1,0));
        glm::mat4 model = glm::mat4(1.0f);

		return proj * view * model;
	}
	
	void PointLight::AddShadow() 
	{
		PL_ASSERT(false, "Point light shadows not implemented.");
	}

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
		m_Lights.push_back(new PointLight(colour, radius, this));
	}

	void LightComponent::AddDirectionalLight(glm::vec3 colour, glm::vec3 target, bool shadow)
	{
		m_Lights.push_back(new DirectionalLight(colour, target, this));

		if (shadow)
		{
			m_Lights.back()->AddShadow();
		}
	}

	void LightComponent::OnUpdate(Scene* scene)
	{

	}
}
