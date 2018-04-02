#include "GameObject.h"
#include "Component.h"

GameObject* GameObject::AddComponent(Component* component)
{
	component->SetOwner(this);
	m_Components.push_back(component);
	return this;
}

void GameObject::OnUpdate(Scene* scene)
{
	for (Component* component : m_Components)
	{
		component->OnUpdate(scene);
	}
}
