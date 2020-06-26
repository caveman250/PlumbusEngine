#include "plumbus.h"
#include "imgui_impl/Log.h"
#include "BaseApplication.h"

// #undef APIENTRY
// #include <windows.h>

#define INFO_COLOUR 15
#define WARN_COLOUR 14
#define ERROR_COLOUR 12
#define FATAL_COLOUR 79

namespace plumbus
{
	bool Log::m_ScrollToBottom = false;
	int Log::s_LogEntryIndex = 0;
	Log::ImGuiLogEntry Log::s_Buffer[s_NumLogMessagesToStore] = { };

	int GetWin32TerminalColour(LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Info:
			return INFO_COLOUR;
		case LogLevel::Warn:
			return WARN_COLOUR;
		case LogLevel::Error:
			return ERROR_COLOUR;
		case LogLevel::Fatal:
			return FATAL_COLOUR;
		default:
			PLUMBUS_ASSERT(false);
			return -1;
		}
	}

	ImVec4 GetImGuiTerminalColour(LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Info:
			return ImVec4(1, 1, 1, 1);
		case LogLevel::Warn:
			return ImVec4(1, 1, 0, 1);
		case LogLevel::Error:
			return ImVec4(1, 0, 0, 1);
		case LogLevel::Fatal:
			return ImVec4(1, 0, 1, 1);
		default:
			PLUMBUS_ASSERT(false);
			return ImVec4(0, 0, 0, 1);
		}
	}

	void Log::Clear()
	{
		for (int i = 0; i < s_NumLogMessagesToStore; ++i)
		{
			s_Buffer[i].message = "";
		}
	}

	void Log::LogInternal(LogLevel level, const char* fmt, ...)
	{
		//HANDLE hConsole;
		//hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		//SetConsoleTextAttribute(hConsole, GetWin32TerminalColour(level));

		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, 1024, fmt, args);
		printf(buffer);
		s_Buffer[s_LogEntryIndex] = ImGuiLogEntry{ (const char*)&buffer, GetImGuiTerminalColour(level) };
		s_LogEntryIndex = ++s_LogEntryIndex % s_NumLogMessagesToStore;

		va_end(args);

		m_ScrollToBottom = true;
	}

	void Log::Info(const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, 1024, fmt, args);

		LogInternal(LogLevel::Info, "[Info] %s%s", &buffer, "\n");

		va_end(args);

	}

	void Log::Warn(const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, 1024, fmt, args);

		LogInternal(LogLevel::Warn, "[Warn] %s%s", &buffer, "\n");

		va_end(args);
	}

	void Log::Error(const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, 1024, fmt, args);

		LogInternal(LogLevel::Error, "[Error] %s%s", &buffer, "\n");

		va_end(args);
	}

	void Log::Fatal(const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, 1024, fmt, args);

		LogInternal(LogLevel::Fatal, "[Fatal] %s%s", &buffer, "\n");

		va_end(args);

		PLUMBUS_ASSERT(false);
	}

	void Log::Draw(const char* title)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin(title);
		int scrollY = -1;

		if (ImGui::Button("Clear")) Clear();
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::Separator();
		ImGui::BeginChild("scrolling");
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
		if (copy) ImGui::LogToClipboard();

		for (int i = 0; i < s_NumLogMessagesToStore; i++)
		{
			int index = (s_LogEntryIndex + i) % s_NumLogMessagesToStore;
			if (s_Buffer[index].message.length() > 0)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, s_Buffer[index].textColour);
				ImGui::TextWrapped(s_Buffer[index].message.c_str());
				ImGui::PopStyleColor();
			}
		}

		if (m_ScrollToBottom)
			ImGui::SetScrollHere(1.0f);

		m_ScrollToBottom = false;
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::End();
	}
}
