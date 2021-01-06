#include "plumbus.h"
#include "MonoManager.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>

#include "Class.h"
#include <mono/metadata/class.h>
#include <filesystem>

namespace plumbus::mono
{
	MonoManager* MonoManager::s_Instance = nullptr;
	
	MonoManager* MonoManager::Get()
	{
		if(!s_Instance)
		{
			s_Instance = new MonoManager();
		}

		return s_Instance;
	}

	void MonoManager::Init()
	{
	    Log::Info("Init mono");
#if PLUMBUS_MONO_DEBUG
		int jitArgC = 1;
		char* jitArgV[1] = {(char*)"--debugger-agent=transport=dt_socket,address=127.0.0.1:50000"};
		mono_jit_parse_options(jitArgC, jitArgV);
		mono_debug_init(MONO_DEBUG_FORMAT_MONO);
        m_Domain = mono_jit_init("PlumbusMono");
#else
        m_Domain = mono_jit_init("PlumbusMono");
#endif
		std::filesystem::path gamePath = "./PlumbusTesterMono.dll";
		std::string absoluteGamePath = absolute(gamePath).string();
		m_GameAssembly = mono_domain_assembly_open (m_Domain, absoluteGamePath.c_str());
		PL_ASSERT(m_GameAssembly, "You probably need to build the C# PlumbusTester project");
		m_GameImage = mono_assembly_get_image(m_GameAssembly);

		std::filesystem::path enginePath = "PlumbusEngineMono.dll";
		std::string absoluteEnginePath = absolute(gamePath).string();
		m_EngineAssembly = mono_domain_assembly_open (m_Domain, absoluteEnginePath.c_str());
		PL_ASSERT(m_GameAssembly, "You probably need to build the C# Engine project");
		m_EngineImage = mono_assembly_get_image(m_EngineAssembly);
	}

	ClassRef MonoManager::GetMonoClass(std::string classNamespace, std::string className)
	{
        Log::Info("Get mono class %s:%s", classNamespace.c_str(), className.c_str());
		MonoClassKey key = { classNamespace, className };
		if (m_Classes.count(key) == 0)
		{
			m_Classes[key] = std::make_shared<Class>(mono_class_from_name(m_GameImage, key.m_NameSpace.c_str(), key.m_ClassName.c_str()));
		}

		return m_Classes[key];
	}

	ClassRef MonoManager::GetMonoEngineClass(std::string classNamespace, std::string className)
	{
        Log::Info("Get mono engine class %s:%s", classNamespace.c_str(), className.c_str());
		MonoClassKey key = { classNamespace, className };
		if (m_EngineClasses.count(key) == 0)
		{
			ClassRef monoClass = std::make_shared<Class>(mono_class_from_name(m_EngineImage, key.m_NameSpace.c_str(), key.m_ClassName.c_str()));
			m_EngineClasses[key] = monoClass;
		}

		return m_EngineClasses[key];
	}

	ObjectInstanceRef MonoManager::CreateMonoObject(ClassRef objClass, int numArgs, void** args)
	{
        Log::Info("Allocating mono class instance 0x%x", objClass.get());
        MonoObject* obj = mono_object_new(m_Domain, objClass->GetMonoClass());
		if (numArgs == 0)
		{
			return std::make_shared<ObjectInstance>(obj, objClass);
		}
		else
		{
			return std::make_shared<ObjectInstance>(obj, objClass, numArgs, args);
		}
	}
}
