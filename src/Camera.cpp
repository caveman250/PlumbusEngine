#include "Camera.h"
#include "Application.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void Camera::Init()
{
	m_Position = glm::vec3(0, -1, -6);
	m_Rotation = glm::vec3(0, 0, 0);
	m_ProjectionMatrix = glm::perspective(glm::radians(60.0f), Application::Get().GetSwapChainExtent().width / (float)Application::Get().GetSwapChainExtent().height, 0.1f,  256.0f);
}

void Camera::OnUpdate()
{
	double xpos, ypos;
	glfwGetCursorPos(Application::Get().GetWindow(), &xpos, &ypos);
	double dx = m_MousePos.x - xpos;
	double dy = m_MousePos.y - ypos;
	m_MousePos = glm::vec2(xpos, ypos);
	float deltaTime = (float)Application::Get().GetDeltaTime();

	if (glfwGetMouseButton(Application::Get().GetWindow(), GLFW_MOUSE_BUTTON_1))
	{
		m_Rotation += glm::vec3(dy * 1.0f, -dx * 1.0f, 0.0f);
	}

	glm::mat4 rotM = glm::mat4(1.0f);
	glm::mat4 transM;

	rotM = glm::rotate(rotM, glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	
	glm::vec3 forward(rotM[0][2], rotM[1][2], rotM[2][2]);

	if (glfwGetKey(Application::Get().GetWindow(), GLFW_KEY_W))
	{
		m_Position += forward * deltaTime * 5.f;
	}
	if (glfwGetKey(Application::Get().GetWindow(), GLFW_KEY_S))
	{
		m_Position -= forward * deltaTime * 5.f;
	}
	if (glfwGetKey(Application::Get().GetWindow(), GLFW_KEY_A))
	{
		m_Position -= glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f))) * deltaTime * 5.f;
	}
	if (glfwGetKey(Application::Get().GetWindow(), GLFW_KEY_D))
	{
		m_Position += glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f))) * deltaTime * 5.f;
	}

	transM = glm::translate(glm::mat4(1.0f), m_Position);

	m_ViewMatrix = rotM * transM;
}
