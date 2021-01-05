#pragma once
#include <string>
#include <mono/metadata/object-forward.h>
#include "mono_fwd.h"

namespace plumbus::mono
{
	class Class;
	class ObjectInstance
	{
	public:
		ObjectInstance(MonoObject* object, ClassRef objClass);
		ObjectInstance(MonoObject* object, ClassRef objClass, int numArgs, void** args);

		MonoObject* GetMonoObject() { return m_Object; }
		void CallMethod(std::string methodName, int argCount = 0, void** args = nullptr);
	private:
		MonoObject* m_Object;
		ClassRef m_Class;
	
	};
}
