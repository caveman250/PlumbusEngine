#pragma once
#include "tests/Test.h"

namespace plumbus::tester::tests
{
	class Scripts : public Test
	{
	public:
		Scripts();
		void Init() override;
		void Update() override;
		void Shutdown() override;
		void OnGui() override;
	private:
		vk::MaterialRef m_DeferredLightMaterial;
	};
}
