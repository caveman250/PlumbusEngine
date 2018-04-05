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

	glm::vec3 m_Position = glm::vec3();
	glm::vec3 m_Rotation = glm::vec3();

	float rotationSpeed = 1.0f;
	float movementSpeed = 1.0f;

	glm::vec2 m_MousePos = glm::vec2(400, 300);
};