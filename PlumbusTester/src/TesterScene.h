#pragma once
#include "Scene.h"

namespace plumbus::tester
{
	class TesterScene : public Scene
	{
	public:
		TesterScene();

		void Init() override;
		void Shutdown() override;
		void OnUpdate() override;
	};
}