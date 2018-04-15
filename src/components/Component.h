#pragma once

class GameObject;
class Scene;

class Component
{
public:
	enum ComponentType
	{
		ModelComponent,
		PointLightComponent,
		TranslationComponent,
		Count
	};

	Component() { }
	virtual void OnUpdate(Scene* scene) = 0;

	void SetOwner(GameObject* owner) { m_Owner = owner; }
	GameObject* GetOwner() { return m_Owner; }

	static const ComponentType GetType() { return Component::Count; }

private:
	GameObject* m_Owner;
};
