#pragma once
#include <map>

#include "mono_impl/mono_fwd.h"

typedef struct _MonoDomain MonoDomain;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage MonoImage;

namespace plumbus::mono
{
	class MonoManager
	{
	public:
		static MonoManager* Get();
		
		MonoManager()
			: m_Domain(nullptr)
			, m_GameAssembly(nullptr)
			, m_GameImage(nullptr)
			, m_EngineAssembly(nullptr)
            , m_EngineImage(nullptr)
		{
		}
		
		void Init();
		ClassRef GetMonoClass(std::string classNamespace, std::string className);
		ClassRef GetMonoEngineClass(std::string classNamespace, std::string className);
		ObjectInstanceRef CreateMonoObject(ClassRef objClass, int numArgs = 0, void** args = nullptr);

	private:
		MonoDomain* m_Domain;
		MonoAssembly* m_GameAssembly;
		MonoImage* m_GameImage;
		MonoAssembly* m_EngineAssembly;
		MonoImage* m_EngineImage;

		struct MonoClassKey
		{
			std::string m_NameSpace;
			std::string m_ClassName;

			bool operator < (const MonoClassKey& other) const
			{
				if (m_NameSpace != other.m_NameSpace)
				{
					return m_NameSpace < other.m_NameSpace;
				}
				if (m_ClassName != other.m_ClassName)
				{
					return m_ClassName < other.m_ClassName;
				}
				
				return false;
			}
		};
		
		std::map<MonoClassKey, ClassRef> m_EngineClasses;
		std::map<MonoClassKey, ClassRef> m_Classes;

		static MonoManager* s_Instance;
	};
}
