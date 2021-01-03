#pragma once

namespace plumbus::tester
{
	namespace tests
	{
		class Test;
	}
	
	class TestManager
	{
	public:
		TestManager();

		static TestManager& Get() { return s_Instance; }

		void Update();
		void OnGui();

		template <typename T>
		void BeginTest();

		void ShutdownActiveTest();

	private:
		void BeginTestInternal();

		tests::Test* m_ActiveTest;

		static TestManager s_Instance;
	};

	template <typename T>
	void TestManager::BeginTest()
	{
		if (m_ActiveTest)
		{
			ShutdownActiveTest();
		}

		m_ActiveTest = new T();
		BeginTestInternal();
	}
}