#pragma once
#include "Shadow.h"

namespace plumbus::vk
{
	class ShadowManager
	{
	public:
		static ShadowManager* Get();
		static void Destroy();
		void RegisterShadow(Shadow* shadow);
		void UnregisterShadow(Shadow* shadow);

		std::vector<Shadow*>& GetShadows() { return m_Shadows; }
	private:
		static ShadowManager* s_Instance;
		
		std::vector<Shadow*> m_Shadows;
	};
}
