#pragma once
#include <gui/GuiTool.h>

namespace plumbus::tester
{
	namespace tests
	{
		class Test;
	}
	
    class TestManager : public gui::GuiTool
	{
	public:
		TestManager();
		static TestManager& Get() { return s_Instance; }

		void Update() override;
		void OnGui() override;

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