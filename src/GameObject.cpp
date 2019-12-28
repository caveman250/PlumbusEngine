#include "plumbus.h"

#include "GameObject.h"
#include "components/GameComponent.h"
#include "components/ModelComponent.h"

namespace plumbus
{
	void GameObject::OnUpdate(Scene* scene)
	{
		for (auto kvp : m_Components)
		{
			kvp.second->OnUpdate(scene);
		}
	}
}
