#pragma once
#include "plumbus.h"
#include "components/GameComponent.h"
#include "components/ModelComponent.h"

namespace plumbus::vk
{
	class Model;
}
namespace plumbus
{
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
		std::map<const GameComponent::ComponentType, GameComponent*> m_Components;
		std::string m_ID;
	};

	template <typename T>
	T* GameObject::GetComponent()
	{
		auto it = m_Components.find(T::GetType());
		if (it != m_Components.end())
			return static_cast<T*>((*it).second);

		return nullptr;
	}

	template <typename T>
	GameObject* GameObject::AddComponent(T* component)
	{
		component->SetOwner(this);
		m_Components[T::GetType()] = component;
		return this;
	}
}