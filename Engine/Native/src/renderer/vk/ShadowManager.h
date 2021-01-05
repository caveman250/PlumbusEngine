#pragma once
#include "Shadow.h"

namespace plumbus::vk
{
	class ShadowManager
	{
	public:
		static ShadowManager* Get();
		static void Destroy();

		ShadowManager();
		void RegisterShadow(ShadowDirectional* shadow);
		void UnregisterShadow(ShadowDirectional* shadow);
        void RegisterShadow(ShadowOmniDirectional* shadow);
        void UnregisterShadow(ShadowOmniDirectional* shadow);

        bool ShadowTexturesOutOfDate();
        void SetShadowTexturesUpToDate();

		std::vector<ShadowDirectional*>& GetDirectionalShadows() { return m_DirShadows; }
        std::vector<ShadowOmniDirectional*>& GetOmniDirectionalShadows() { return m_OmniDirShadows; }
	private:
		static ShadowManager* s_Instance;
		
		std::vector<ShadowDirectional*> m_DirShadows;
        std::vector<ShadowOmniDirectional*> m_OmniDirShadows;

        bool m_ShadowTexturesUpToDate;
	};
}
