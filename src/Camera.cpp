#include "Camera.h"
#include "Application.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "renderer/base/Renderer.h"
#include "renderer/base/Window.h"

#if VULKAN_RENDERER
#include "renderer/vk/VulkanRenderer.h"
#endif

void Camera::Init()
{
	m_Position = glm::vec3(0, 1, -8);
#if VULKAN_RENDERER
	m_Rotation = glm::vec3(0, 0, 0);
#elif METAL_RENDERER
	m_Rotation = glm::vec3(0, 0, 180);
#endif
    
    m_ProjectionMatrix = glm::perspective(glm::radians(60.0f), Application::Get().GetRenderer()->GetWindow()->GetWidth() / (float)Application::Get().GetRenderer()->GetWindow()->GetHeight(), 0.1f,  256.0f);
}

void Camera::OnUpdate()
{
	glm::vec2 mousePos = Application::Get().GetRenderer()->GetWindow()->GetMousePos();
	double dx = m_MousePos.x - mousePos.x;
	double dy = m_MousePos.y - mousePos.y;
	m_MousePos = mousePos;
	float deltaTime = (float)Application::Get().GetDeltaTime();

#if VULKAN_RENDERER //TODO
	vk::VulkanRenderer* renderer = static_cast<vk::VulkanRenderer*>(Application::Get().GetRenderer());
	bool focused = Application::Get().m_GameFocued;
	bool mouseDown = glfwGetMouseButton(renderer->GetWindow(), GLFW_MOUSE_BUTTON_1);
	if (focused && mouseDown)
#endif
	{
			m_Rotation += glm::vec3(dy * 1.0f, -dx * 1.0f, 0.0f);
	}

	glm::mat4 rotM = glm::mat4(1.0f);
	glm::mat4 transM;

	rotM = glm::rotate(rotM, glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	
#if VULKAN_RENDERER //TODO
	glm::vec3 forward(rotM[0][2], rotM[1][2], rotM[2][2]);

	if (glfwGetKey(renderer->GetWindow(), GLFW_KEY_W))
	{
		m_Position += forward * deltaTime * 5.f;
	}
	if (glfwGetKey(renderer->GetWindow(), GLFW_KEY_S))
	{
		m_Position -= forward * deltaTime * 5.f;
	}
	if (glfwGetKey(renderer->GetWindow(), GLFW_KEY_A))
	{
		m_Position -= glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f))) * deltaTime * 5.f;
	}
	if (glfwGetKey(renderer->GetWindow(), GLFW_KEY_D))
	{
		m_Position += glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f))) * deltaTime * 5.f;
	}
#endif
    
	transM = glm::translate(glm::mat4(1.0f), m_Position);

	m_ViewMatrix = rotM * transM;
}
