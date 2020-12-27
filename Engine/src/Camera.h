#pragma once
#include "glm/glm.hpp"

namespace plumbus
{
	class Camera
	{
	public:
		void Init();
		void OnUpdate();

		glm::mat4 GetViewMatrix() { return m_ViewMatrix; }
		glm::mat4 GetProjectionMatrix() { return m_ProjectionMatrix; }
		glm::vec3 GetPosition() { return m_Position; }
		glm::vec3 GetRotation() { return m_Rotation; }

		void SetViewMatrix(glm::mat4 viewMat) { m_ViewMatrix = viewMat; }
		void SetProjectionMatrix(glm::mat4 projMat) { m_ProjectionMatrix = projMat; }
		void SetPosition(glm::vec3 pos) { m_Position = pos; }
		void SetRotation(glm::vec3 rot) { m_Rotation = rot; }

		void OnGui();

	private:
		glm::mat4	m_ViewMatrix;
		glm::mat4	m_ProjectionMatrix;

		glm::vec3 m_Position = glm::vec3();
		glm::vec3 m_Rotation = glm::vec3();

		float rotationSpeed = 1.0f;
		float movementSpeed = 1.0f;

		glm::vec2 m_MousePos = glm::vec2(400, 300);
	};
}