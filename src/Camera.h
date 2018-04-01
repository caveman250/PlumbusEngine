#pragma once
#include "glm/glm.hpp"

class Camera
{
public:
	void Init();
	void OnUpdate();

	glm::mat4	GetViewMatrix() { return m_ViewMatrix; }
	glm::mat4	GetProjectionMatrix() { return m_ProjectionMatrix; }

private:
	glm::mat4	m_ViewMatrix;
	glm::mat4	m_ProjectionMatrix;

	glm::vec3 m_CameraPosition = glm::vec3(0, 0, 0);

	float m_HorizontalAngle = 20.0f;
	float m_VerticalAngle = -145.0f;
	glm::vec2 m_MousePos = glm::vec2(400, 300);
};