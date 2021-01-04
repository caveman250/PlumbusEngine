#pragma once
#include "plumbus.h"
#include "components/GameComponent.h"
#include "components/ModelComponent.h"

namespace plumbus::vk
{
	class Mesh;
}
namespace plumbus
{
	namespace components
	{
		class ModelComponent;
	}
	
	class Scene;
	class GameObject
	{
	public:
		GameObject(std::string id);
		~GameObject();

		std::string GetID() { return m_ID; }

		template <typename T>
		GameObject* AddComponent(T* component);

		void OnUpdate(Scene* scene);
		void Init();
		void PostInit();

		template <typename T>
		T* GetComponent();

	private:
		std::map<const components::GameComponent::ComponentType, components::GameComponent*> m_Components;
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

#include "mono_impl/mono_fwd.h"
enum ComponentType
{
    TranslationComponent,
};
MONO_EXPORT(uint64_t) GetComponent(uint64_t obj, ComponentType type);