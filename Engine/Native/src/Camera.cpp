#include "plumbus.h"

#include "Camera.h"
#include "BaseApplication.h"
#include "renderer/vk/VulkanRenderer.h"
#include "renderer/vk/Window.h"
#include <glm/gtx/rotate_vector.hpp>


#include "Scene.h"
#include "../../../PlumbusTester/src/Application.h"
#include "platform/Input.h"

namespace plumbus
{
	void Camera::Init()
	{
		m_Position = glm::vec3(23, 14.5, 0);
		m_Rotation = glm::vec3(-18, 0, 90);
		m_ProjectionMatrix = glm::perspective(glm::radians(30.0f), BaseApplication::Get().GetRenderer()->GetWindow()->GetWidth() / (float)BaseApplication::Get().GetRenderer()->GetWindow()->GetHeight(), 0.1f, 256.0f);
	}

	void Camera::OnUpdate()
	{
		glm::mat4 rotM = glm::mat4(1.0f);
		glm::mat4 transM;

		glm::mat4 rotMX = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 rotMY = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 rotMZ = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		rotM = rotMX * rotMZ * rotMY;
		transM = glm::translate(glm::mat4(1.0f), m_Position);

		m_ViewMatrix = rotM * transM;

#if PL_PLATFORM_ANDROID
		glm::mat4 preTransform = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0, 0, 1));
		m_ViewMatrix = preTransform * m_ViewMatrix;
#endif
	}

	void Camera::OnGui()
	{
		ImGui::Text("Camera:");
		ImGui::DragFloat3("Rotation", (float*)&m_Rotation);
	}

    glm::vec3 Camera::GetForward()
    {
        return glm::vec3(m_ViewMatrix[0][2], m_ViewMatrix[1][2], m_ViewMatrix[2][2]);
    }
}

mono_mat4 Camera_GetViewMatrix()
{
	return plumbus::tester::Application::Get().GetScene()->GetCamera()->GetViewMatrix();
}

mono_mat4 Camera_GetProjectionMatrix()
{
	return plumbus::tester::Application::Get().GetScene()->GetCamera()->GetProjectionMatrix();
}

mono_vec3 Camera_GetPosition()
{
	return plumbus::tester::Application::Get().GetScene()->GetCamera()->GetPosition();
}

mono_vec3 Camera_GetRotation()
{
	return plumbus::tester::Application::Get().GetScene()->GetCamera()->GetRotation();
}

void Camera_SetViewMatrix(mono_mat4 viewMat)
{
	plumbus::tester::Application::Get().GetScene()->GetCamera()->SetViewMatrix(glm::mat4(viewMat.x0, viewMat.y0, viewMat.z0, viewMat.w0,
																								viewMat.x1, viewMat.y1, viewMat.z1, viewMat.w1,
																								viewMat.x2, viewMat.y2, viewMat.z2, viewMat.w2,
																								viewMat.x3, viewMat.y3, viewMat.z3, viewMat.w3));
}

void Camera_SetProjectionMatrix(mono_mat4 projMat)
{
	plumbus::tester::Application::Get().GetScene()->GetCamera()->SetViewMatrix(glm::mat4(projMat.x0, projMat.y0, projMat.z0, projMat.w0,
																								projMat.x1, projMat.y1, projMat.z1, projMat.w1,
																								projMat.x2, projMat.y2, projMat.z2, projMat.w2,
																								projMat.x3, projMat.y3, projMat.z3, projMat.w3));
}

void Camera_SetPosition(mono_vec3 pos)
{
	plumbus::tester::Application::Get().GetScene()->GetCamera()->SetPosition({pos.x, pos.y, pos.z});
}

void Camera_SetRotation(mono_vec3 rot)
{
	plumbus::tester::Application::Get().GetScene()->GetCamera()->SetRotation({rot.x, rot.y, rot.z});
}

mono_vec3 Camera_GetForward()
{
    return plumbus::tester::Application::Get().GetScene()->GetCamera()->GetForward();
}
