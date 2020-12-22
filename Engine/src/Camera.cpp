#include "plumbus.h"

#include "Camera.h"
#include "BaseApplication.h"
#include "renderer/vk/VulkanRenderer.h"
#include "renderer/vk/Window.h"
#include <glm/gtx/rotate_vector.hpp>

namespace plumbus
{
	void Camera::Init()
	{
		m_Position = glm::vec3(23, 14.5, 0);
		m_Rotation = glm::vec3(-18, 0, 90);
		m_ProjectionMatrix = glm::perspective(glm::radians(60.0f), BaseApplication::Get().GetRenderer()->GetWindow()->GetWidth() / (float)BaseApplication::Get().GetRenderer()->GetWindow()->GetHeight(), 0.1f, 256.0f);
	}

	void Camera::OnUpdate()
	{
		glm::vec2 mousePos = BaseApplication::Get().GetRenderer()->GetWindow()->GetMousePos();
		double dx = m_MousePos.x - mousePos.x;
		double dy = m_MousePos.y - mousePos.y;
		m_MousePos = mousePos;
		float deltaTime = (float)BaseApplication::Get().GetDeltaTime();

		vk::VulkanRenderer* renderer = vk::VulkanRenderer::Get();
#if !PL_DIST
		bool focused = BaseApplication::Get().m_GameWindowFocused;
#else
		bool focused = true;
#endif

#if !PL_PLATFORM_ANDROID //TODO
		bool mouseDown = glfwGetMouseButton(renderer->GetWindow()->GetWindow(), GLFW_MOUSE_BUTTON_1);
		if (focused && mouseDown)
		{
			m_Rotation += glm::vec3(dy * 1.0f, -dx * 1.0f, 0.0f);
		}
#endif

		glm::mat4 rotM = glm::mat4(1.0f);
		glm::mat4 transM;

		glm::mat4 rotMX = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 rotMY = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 rotMZ = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 forward(rotM[0][2], rotM[1][2], rotM[2][2]);
#if !PL_PLATFORM_ANDROID
		if (glfwGetKey(renderer->GetWindow()->GetWindow(), GLFW_KEY_W))
		{
			m_Position += forward * deltaTime * 20.f;
		}
		if (glfwGetKey(renderer->GetWindow()->GetWindow(), GLFW_KEY_S))
		{
			m_Position -= forward * deltaTime * 20.f;
		}
		if (glfwGetKey(renderer->GetWindow()->GetWindow(), GLFW_KEY_A))
		{
			m_Position -= glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f))) * deltaTime * 20.f;
		}
		if (glfwGetKey(renderer->GetWindow()->GetWindow(), GLFW_KEY_D))
		{
			m_Position += glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f))) * deltaTime * 20.f;
		}
#endif

		//m_ViewMatrix = glm::lookAt(m_Position, m_Position + forward, glm::vec3(0, 0, 1));

		transM = glm::translate(glm::mat4(1.0f), m_Position);

		m_ViewMatrix = rotMX * transM;
        m_ViewMatrix = rotMZ * m_ViewMatrix;
        m_ViewMatrix = rotMY * m_ViewMatrix;

#if PL_PLATFORM_ANDROID
		glm::mat4 preTransform = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0, 0, 1));
		m_ViewMatrix = preTransform * m_ViewMatrix;
#endif
	}
}