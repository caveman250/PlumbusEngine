#include "Class.h"

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include "MonoManager.h"
#include "imgui_impl/Log.h"

namespace plumbus::mono
{
	void Class::CallMethod(std::string methodName, int argCount, void** args, ObjectInstance* instance)
	{
		if (m_MethodMap.count(methodName) == 0)
		{
			m_MethodMap[methodName] = mono_class_get_method_from_name(m_Class, methodName.c_str(), argCount);
		}

		mono_runtime_invoke(m_MethodMap[methodName], instance ? instance->GetMonoObject() : nullptr, args, nullptr);
	}
}
