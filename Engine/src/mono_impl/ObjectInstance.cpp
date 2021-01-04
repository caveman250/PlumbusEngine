#include "plumbus.h"
#include "ObjectInstance.h"
#include "Class.h"

#include <mono/metadata/object.h>

namespace plumbus::mono
{
	ObjectInstance::ObjectInstance(MonoObject* object, ClassRef objClass)
		: m_Object(object)
		, m_Class(objClass)
	{
		mono_runtime_object_init(object);
	}

	ObjectInstance::ObjectInstance(MonoObject* object, ClassRef objClass, int numArgs, void** args)
		: m_Object(object)
        , m_Class(objClass)
	{
		objClass->CallMethod(".ctor", numArgs, args, this);
	}

	void ObjectInstance::CallMethod(std::string methodName, int argCount, void** args)
	{
		m_Class->CallMethod(methodName, argCount, args, this);
	}
}
