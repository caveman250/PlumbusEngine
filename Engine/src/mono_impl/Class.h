#pragma once
#include <unordered_map>
#include <mono/metadata/object-forward.h>

#include "mono_fwd.h"
#include "ObjectInstance.h"

namespace plumbus::mono
{
	class Class
	{
	public:
		Class(MonoClass* monoClass)
			: m_Class(monoClass)
		{
		}

		void CallMethod(std::string methodName, int argCount = 0, void** args = nullptr, ObjectInstance* instance = nullptr);

		MonoClass* GetMonoClass() { return m_Class; }
	private:
		MonoClass* m_Class;
		std::unordered_map<std::string, MonoMethod*> m_MethodMap;
	};
}
