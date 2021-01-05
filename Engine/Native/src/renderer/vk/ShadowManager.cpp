#include "ShadowManager.h"

#include "VulkanRenderer.h"

namespace plumbus::vk
{
	ShadowManager* ShadowManager::s_Instance = nullptr;

    ShadowManager::ShadowManager()
            : m_ShadowTexturesUpToDate(false)
    {

    }
	
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

	void ShadowManager::RegisterShadow(ShadowDirectional* shadow)
	{
		m_DirShadows.push_back(shadow);
        m_ShadowTexturesUpToDate = false;
	}

	void ShadowManager::UnregisterShadow(ShadowDirectional* shadow)
	{
        m_DirShadows.erase(std::remove(m_DirShadows.begin(), m_DirShadows.end(), shadow), m_DirShadows.end());
        m_ShadowTexturesUpToDate = false;
	}

    void ShadowManager::RegisterShadow(ShadowOmniDirectional* shadow)
    {
        m_OmniDirShadows.push_back(shadow);
        m_ShadowTexturesUpToDate = false;
    }

    void ShadowManager::UnregisterShadow(ShadowOmniDirectional* shadow)
    {
        m_OmniDirShadows.erase(std::remove(m_OmniDirShadows.begin(), m_OmniDirShadows.end(), shadow), m_OmniDirShadows.end());
        m_ShadowTexturesUpToDate = false;
    }

    bool ShadowManager::ShadowTexturesOutOfDate()
    {
        return !m_ShadowTexturesUpToDate;
    }

    void ShadowManager::SetShadowTexturesUpToDate()
    {
        m_ShadowTexturesUpToDate = true;
    }
}
