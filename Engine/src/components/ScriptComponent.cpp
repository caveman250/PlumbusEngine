#include "plumbus.h"
#include "ScriptComponent.h"


#include "mono_impl/Class.h"
#include "mono_impl/MonoManager.h"
#include "mono_impl/ObjectInstance.h"

namespace plumbus::components
{
	ScriptComponent::ScriptComponent(std::string scriptName, std::string scriptNamespace)
		: GameComponent()
		, m_ScriptName(scriptName)
		, m_ScriptNamespace(scriptNamespace)
		, m_MonoObject(nullptr)
	{
	}

	ScriptComponent::~ScriptComponent()
	{
		m_MonoObject.reset();
	}

	void ScriptComponent::Init()
	{
		m_ScriptName.resize(m_ScriptName.size() - 3);
		mono::ClassRef monoClass = mono::MonoManager::Get()->GetMonoClass(m_ScriptNamespace, m_ScriptName);

		void* args[] = {this, GetOwner() };
		m_MonoObject = mono::MonoManager::Get()->CreateMonoObject(monoClass, 2, args);
		
	}

	void ScriptComponent::OnUpdate(Scene* scene)
	{
		m_MonoObject->CallMethod("Update");
	}
}
