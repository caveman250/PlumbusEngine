#pragma once

#include "tests/Test.h"

namespace plumbus::tester::tests
{
	class DeferredLights : public Test 
	{
	public:
		DeferredLights();
		void Init() override;
		void Update() override;
		void Shutdown() override;
		void OnGui() override;
	private:
		double m_LightTime = 0;
	};
}

