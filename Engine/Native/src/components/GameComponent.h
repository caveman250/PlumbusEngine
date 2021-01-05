#pragma once
namespace plumbus
{
	class GameObject;
	class Scene;
}
namespace plumbus::components
{
	class GameComponent
	{
	public:
		enum ComponentType
		{
			ModelComponent,
			PointLightComponent,
			TranslationComponent,
			Count
		};

		GameComponent() { }
		virtual ~GameComponent() {}

		virtual void Init() = 0;
		virtual void PostInit() = 0;
		virtual void OnUpdate(Scene* scene) = 0;

		void SetOwner(GameObject* owner) { m_Owner = owner; }
		GameObject* GetOwner() { return m_Owner; }

		static const ComponentType GetType() { return GameComponent::Count; }

	private:
		GameObject* m_Owner;
	};
}
