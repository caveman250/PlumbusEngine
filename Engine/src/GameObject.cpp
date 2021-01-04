#include "plumbus.h"

#include "GameObject.h"

#include <mono_impl/mono_fwd.h>

#include "components/GameComponent.h"
#include "components/ModelComponent.h"
#include "mono_impl/Class.h"
#include "mono_impl/MonoManager.h"

namespace plumbus
{
	GameObject::GameObject(std::string id)
	{
		m_ID = id;
		mono::ClassRef monoClass = mono::MonoManager::Get()->GetMonoEngineClass("PlumbusEngine", "GameObject");
		void* args[] = { this };
		monoClass->CallMethod("RegisterGameObject", 1, args);
	}

	GameObject::~GameObject()
	{
		for (auto& [_, component] : m_Components)
		{
			delete component;
		}
	}

	void GameObject::OnUpdate(Scene* scene)
	{
		for (auto& [_, component]: m_Components)
		{
			component->OnUpdate(scene);
		}
	}

	void GameObject::Init()
	{
		for (auto& [_, component]: m_Components)
		{
			component->Init();
		}
	}

	void GameObject::PostInit()
	{
		for (auto& [_, component]: m_Components)
		{
			component->PostInit();
		}
	}
}
