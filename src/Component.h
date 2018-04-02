#pragma once
#include "GameObject.h"

class GameObject;
class Scene;
class Component
{
public:
	Component() {}
	virtual void OnUpdate(Scene* scene) = 0;
	virtual void OnDraw() {}

	void SetOwner(GameObject* owner) { m_Owner = owner; }
	GameObject* GetOwner() { return m_Owner; }

private:
	GameObject* m_Owner;
};
