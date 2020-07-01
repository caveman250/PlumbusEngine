#pragma once

#include "BaseApplication.h"

namespace plumbus::tester
{
	class Application : public BaseApplication
	{
	public:
		Application();

		static void CreateInstance() { s_Instance = new Application(); }
		static Application& Get() { return *static_cast<Application*>(s_Instance); }

		virtual void Cleanup() override;

		virtual void OnGui() override;
	};
}
