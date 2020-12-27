#pragma once

#include "tests/Test.h"

namespace plumbus
{
	namespace base
	{
		class Material;
	}

	namespace tester::tests
	{
		class Shadows : public Test
		{
		public:
			Shadows();
			~Shadows();
			void Init() override;
			void Update() override;
			void Shutdown() override;
			void OnGui() override;
		private:
			vk::MaterialRef m_DeferredLightMaterial;
#if !PL_DIST
			vk::MaterialInstanceRef m_ShadowTextureImGui;
#endif
		};
	}
}

