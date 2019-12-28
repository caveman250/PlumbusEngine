#pragma once

namespace plumbus::tester::tests
{
	class Test
	{
	public:
		virtual void Init() = 0;
		virtual void Update() = 0;
		virtual void Shutdown() = 0;
		virtual void OnGui() = 0;

	protected:
		Test() {}
	};
}