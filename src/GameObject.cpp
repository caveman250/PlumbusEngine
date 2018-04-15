#include "GameObject.h"
#include "components/Component.h"
#include "components/ModelComponent.h"

void GameObject::OnUpdate(Scene* scene)
{
	for (auto kvp : m_Components)
	{
		kvp.second->OnUpdate(scene);
	}
}
