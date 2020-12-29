#include "ShadowManager.h"

#include "VulkanRenderer.h"

namespace plumbus::vk
{
	ShadowManager* ShadowManager::s_Instance = nullptr;
	
	ShadowManager* ShadowManager::Get()
	{
		if(s_Instance == nullptr)
			s_Instance = new ShadowManager();
		return s_Instance;
	}

	void ShadowManager::Destroy()
	{
		if (s_Instance)
		{
			delete s_Instance;
			s_Instance = nullptr;
		}
	}


	void ShadowManager::RegisterShadow(Shadow* shadow)
	{
		m_Shadows.push_back(shadow);
	}

	void ShadowManager::UnregisterShadow(Shadow* shadow)
	{
		m_Shadows.erase(std::remove(m_Shadows.begin(), m_Shadows.end(), shadow), m_Shadows.end());
	}
}
