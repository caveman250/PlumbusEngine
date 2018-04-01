#include "Camera.h"
#include "Application.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void Camera::Init()
{
	m_ViewMatrix = glm::lookAt(m_CameraPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_ProjectionMatrix = glm::perspective(glm::radians(45.0f), Application::Get().GetSwapChainExtent().width / (float)Application::Get().GetSwapChainExtent().height, 0.1f, 10.0f);
}

void Camera::OnUpdate()
{
	double xpos, ypos;
	glfwGetCursorPos(Application::Get().GetWindow(), &xpos, &ypos);
	glfwSetCursorPos(Application::Get().GetWindow(), 800 / 2, 600 / 2);

	float deltaTime = Application::Get().GetDeltaTime();

	m_HorizontalAngle += 0.5 * deltaTime * float(800 / 2 - xpos);
	m_VerticalAngle += 0.5 * deltaTime * float(600 / 2 - ypos);

	glm::vec3 forward(
		cos(m_VerticalAngle) * sin(-m_HorizontalAngle),
		cos(m_VerticalAngle) * cos(-m_HorizontalAngle),
		sin(m_VerticalAngle));

	glm::vec3 right = glm::vec3(
		sin(-m_HorizontalAngle - 3.14f / 2.0f),
		cos(-m_HorizontalAngle - 3.14f / 2.0f),
		0);


	glm::vec3 up = glm::cross(right, forward);

	if (glfwGetKey(Application::Get().GetWindow(), GLFW_KEY_W))
	{
		m_CameraPosition += forward * deltaTime * 5.f;
	}
	if (glfwGetKey(Application::Get().GetWindow(), GLFW_KEY_S))
	{
		m_CameraPosition -= forward * deltaTime * 5.f;
	}
	if (glfwGetKey(Application::Get().GetWindow(), GLFW_KEY_A))
	{
		m_CameraPosition += right * deltaTime * 5.f;
	}
	if (glfwGetKey(Application::Get().GetWindow(), GLFW_KEY_D))
	{
		m_CameraPosition -= right * deltaTime * 5.f;
	}

	m_ViewMatrix = glm::lookAt(m_CameraPosition, m_CameraPosition + forward, glm::vec3(0.0f, 0.0f, 1.0f));
}
