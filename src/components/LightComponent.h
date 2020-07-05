#pragma once
#include "components/GameComponent.h"
#include "glm/glm.hpp"

namespace plumbus
{
	enum class LightType
	{
		Directional,
		Point,
		Invalid
	};

	class Light 
	{
	public:
		Light(glm::vec3 colour) : m_Colour(colour) {}
		~Light() {}
		glm::vec3 GetColour() { return m_Colour; }
		void SetColour(glm::vec3 colour) { m_Colour = colour; }
		LightType GetType() { return m_Type; }
	protected:
		glm::vec3 m_Colour;
		LightType m_Type = LightType::Invalid;
	};
	class DirectionalLight : public Light 
	{
	public:
		DirectionalLight(glm::vec3 colour, glm::vec3 direction) : Light(colour), m_Direction(direction) { m_Type = LightType::Directional; }
		~DirectionalLight() {}

		glm::vec3 GetDirection() { return m_Direction; }
		void SetDirection(glm::vec3 dir) { m_Direction = dir; }
	private:
		glm::vec3 m_Direction;
	};
	class PointLight : public Light 
	{
	public:
		PointLight(glm::vec3 colour, float radius) : Light(colour), m_Radius(radius) { m_Type = LightType::Point; }
		~PointLight() {}

		float GetRadius() { return m_Radius; }
		void SetRadius(float radius) { m_Radius = radius; }
	private:
		float m_Radius;
	};

	class LightComponent : public GameComponent
	{
	public:
		LightComponent();
		~LightComponent();

		void AddPointLight(glm::vec3 colour, float radius);
		void AddDirectionalLight(glm::vec3 colour, glm::vec3 direction);

		void OnUpdate(Scene* scene) override;

		std::vector<Light*>& GetLights() { return m_Lights; }

		static const ComponentType GetType() { return GameComponent::PointLightComponent; }

	private:
		
		std::vector<Light*> m_Lights;
	};

}