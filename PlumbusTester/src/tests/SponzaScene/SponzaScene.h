#pragma once

#include "tests/Test.h"
#include "renderer/base/renderer_fwd.h"

namespace plumbus
{
	namespace base
	{
		class Material;
	}

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

			MaterialRef m_DeferredLightMaterial;
		};
	}
}

