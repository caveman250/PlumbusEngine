#pragma once

#include "renderer/vk/vk_types_fwd.h"
#include "tests/Test.h"

namespace plumbus
{
	namespace tester::tests
	{
		class SponzaScene : public Test
		{
		public:
			SponzaScene();
			~SponzaScene();
			void Init() override;
			void Update() override;
			void Shutdown() override;
			void OnGui() override;
		private:

			vk::MaterialRef m_DeferredLightMaterial;
		};
	}
}

