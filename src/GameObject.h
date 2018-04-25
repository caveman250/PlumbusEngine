#pragma once
#include "glm/glm.hpp"
#include <vector>
#include <map>
#include "components/Component.h"

namespace vk
{
	class Model;
}
class Scene;
class ModelComponent;
class GameObject
{
public:
	GameObject(std::string id) { m_ID = id; }
	std::string GetID() { return m_ID; }

	template <typename T>
	GameObject* AddComponent(T* component);

	void OnUpdate(Scene* scene);

	template <typename T>
	T* GetComponent();

private:
	std::map<const Component::ComponentType, Component*> m_Components;
	std::string m_ID;
};

template <typename T>
T* GameObject::GetComponent()
{
	if (m_Components.find(T::GetType()) != m_Components.end())
		return static_cast<T*>(m_Components[T::GetType()]);

	return nullptr;
}

template <typename T>
GameObject* GameObject::AddComponent(T* component)
{
	component->SetOwner(this);
	m_Components[T::GetType()] = component;
	return this;
}
