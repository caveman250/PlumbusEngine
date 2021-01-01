#pragma once
#include "components/GameComponent.h"
#include "glm/glm.hpp"
#include "renderer/vk/Shadow.h"

namespace plumbus
{
	enum class LightType
	{
		Directional,
		Point,
		Invalid
	};

	class LightComponent;
	class Light 
	{
	public:
		Light(glm::vec3 colour, LightComponent* parent) : m_Colour(colour), m_Shadow(nullptr), m_LightComponent(parent) {}
		virtual ~Light() {}
		glm::vec3 GetColour() { return m_Colour; }
		void SetColour(glm::vec3 colour) { m_Colour = colour; }
		LightType GetType() { return m_Type; }

		vk::ShadowRef GetShadow() { return m_Shadow; }
		virtual void AddShadow() = 0;
		LightComponent* GetParent() { return m_LightComponent; }
		void RemoveShadow() { if (m_Shadow) { m_Shadow.reset(); } }
	protected:
		glm::vec3 m_Colour;
		LightType m_Type = LightType::Invalid;
		vk::ShadowRef m_Shadow;
		LightComponent* m_LightComponent;
	};

	class DirectionalLight : public Light 
	{
	public:
		DirectionalLight(glm::vec3 colour, glm::vec3 direction, LightComponent* parent) : Light(colour, parent), m_Direction(direction) { m_Type = LightType::Directional; }
		~DirectionalLight() {}

		glm::vec3 GetDirection() { return m_Direction; }
		void SetDirection(glm::vec3 dir) { m_Direction = dir; }
		void AddShadow() override;
		glm::mat4 GetMVP();
	private:
		glm::vec3 m_Direction;
	};

	class PointLight : public Light 
	{
	public:
		PointLight(glm::vec3 colour, float radius, LightComponent* parent) : Light(colour, parent), m_Radius(radius) { m_Type = LightType::Point; }
		~PointLight() {}

		float GetRadius() { return m_Radius; }
		void SetRadius(float radius) { m_Radius = radius; }
		void AddShadow() override;
	private:
		float m_Radius;
	};

	class LightComponent : public GameComponent
	{
	public:
		LightComponent();
		~LightComponent();

		void AddPointLight(glm::vec3 colour, float radius, bool shadow);
		void AddDirectionalLight(glm::vec3 colour, glm::vec3 target, bool shadow);

		void OnUpdate(Scene* scene) override;

		std::vector<Light*>& GetLights() { return m_Lights; }

		static const ComponentType GetType() { return GameComponent::PointLightComponent; }

	private:
		
		std::vector<Light*> m_Lights;
	};

}