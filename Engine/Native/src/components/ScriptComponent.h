#pragma once
#include "GameComponent.h"
#include "mono_impl/mono_fwd.h"

namespace plumbus
{
	class Scene;
}
namespace plumbus::components
{
	class ScriptComponent : public GameComponent
	{
	public:
		ScriptComponent(std::string scriptName, std::string scriptNamespace);
		virtual ~ScriptComponent();

		void Init() override;
		void PostInit() override {}
		void OnUpdate(Scene* scene) override;
	private:
		std::string m_ScriptNamespace;
		std::string m_ScriptName;
		mono::ObjectInstanceRef m_MonoObject;
	};
}
