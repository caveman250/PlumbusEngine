#pragma once
#include "imgui/imgui.h"
namespace plumbus
{
	enum class LogLevel;
	class Log
	{
	public:

		static void Clear();
		static void Info(const char* fmt, ...) IM_FMTARGS(2);
		static void Warn(const char* fmt, ...) IM_FMTARGS(2);
		static void Error(const char* fmt, ...) IM_FMTARGS(2);
		static void Fatal(const char* fmt, ...) IM_FMTARGS(2);

		static void Draw(const char* title);

	private:
		static void LogInternal(LogLevel level, const char* fmt, ...) IM_FMTARGS(2);

		static bool m_ScrollToBottom;
		static int s_LogEntryIndex;

		struct ImGuiLogEntry
		{
			std::string message;
			ImVec4 textColour;
		};

		static constexpr int s_NumLogMessagesToStore = 500;
		static ImGuiLogEntry s_Buffer[s_NumLogMessagesToStore];
	};

	enum class LogLevel
	{
		Info,
		Warn,
		Error,
		Fatal
	};
}