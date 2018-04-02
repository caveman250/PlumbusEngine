#pragma once
#include "glm/glm.hpp"
#include <vector>
#include "Component.h"
#include "ModelComponent.h"

class Model;
class Scene;
class Component;
class GameObject
{
public:
	GameObject() {}

	glm::vec3 GetPosition() { return m_Position; }
	glm::vec3 GetRotation() { return m_Rotation; }
	GameObject* AddComponent(Component* component);

	void OnUpdate(Scene* scene);

	template <typename T>
	T* GetComponent();

private:
	Model * model;
	glm::vec3 m_Position;
	glm::vec3 m_Rotation;

	std::vector<Component*> m_Components;
};

template <typename T>
T* GameObject::GetComponent()
{
	for (Component* component : m_Components)
	{
		if (T* castComponent = dynamic_cast<T*>(component))
			return castComponent;
	}

	return nullptr;
}
