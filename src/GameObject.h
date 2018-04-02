#pragma once
#include "glm/glm.hpp"
#include <vector>

class Model;
class Scene;
class Component;
class ModelComponent;
class GameObject
{
public:
	GameObject() {}

	GameObject* AddComponent(Component* component);

	void OnUpdate(Scene* scene);

	template <typename T>
	T* GetComponent();

private:
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
