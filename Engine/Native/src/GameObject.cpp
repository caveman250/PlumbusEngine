#include "plumbus.h"

#include "GameObject.h"

#include <mono_impl/mono_fwd.h>
#include <TranslationComponent.h>

#include "components/GameComponent.h"
#include "components/ModelComponent.h"
#include "mono_impl/Class.h"
#include "mono_impl/MonoManager.h"

namespace plumbus
{
	GameObject::GameObject(std::string id)
	{
		m_ID = id;
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

uint64_t GetComponent(uint64_t obj, ComponentType type)
{
    plumbus::GameObject* objPtr = reinterpret_cast<plumbus::GameObject*>(obj);
    switch (type)
    {
    	case TranslationComponent:
    		return reinterpret_cast<uint64_t>(objPtr->GetComponent<plumbus::components::TranslationComponent>());
    }

    return 0;
}
