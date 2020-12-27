#include "plumbus.h"
#include "imgui_impl/Log.h"
#include "BaseApplication.h"
#if PL_PLATFORM_ANDROID
#include <android/log.h>
#endif

#define BEGIN_YELLOW printf("\033[0;33m");
#define BEGIN_WHITE printf("\033[0;37m");
#define BEGIN_RED printf("\033[0;31m");
#define BEGIN_FATAL printf("\033[0;41m");

#define END_COLOUR printf("\033[0m\n");

namespace plumbus
{
	bool Log::m_ScrollToBottom = false;
	int Log::s_LogEntryIndex = 0;
	Log::ImGuiLogEntry Log::s_Buffer[s_NumLogMessagesToStore] = { };

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
			PL_ASSERT(false);
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
		char buffer[1024];
		va_list args;
        va_start(args, fmt);
		vsnprintf(buffer, 1024, fmt, args);
#if PL_PLATFORM_ANDROID
		((void)__android_log_print(ANDROID_LOG_INFO, "PlumbusEngine", "%s", buffer));
#else
		printf("%s", buffer);
		fflush(stdout);
#endif
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
		//BEGIN_WHITE
		LogInternal(LogLevel::Info, "[Info] %s\n", (char*)&buffer);
		//END_COLOUR

		va_end(args);

	} 

	void Log::Warn(const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, 1024, fmt, args);
		BEGIN_YELLOW
		LogInternal(LogLevel::Warn, "[Warn] %s",(char*)&buffer);
		END_COLOUR

		va_end(args);
	}

	void Log::Error(const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, 1024, fmt, args);

		BEGIN_RED
		LogInternal(LogLevel::Error, "[Error] %s", (char*)&buffer);
		END_COLOUR

		va_end(args);
	}

	void Log::Fatal(const char* fmt, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, 1024, fmt, args);

		BEGIN_FATAL
		LogInternal(LogLevel::Fatal, "[Fatal] %s", (char*)&buffer);
		END_COLOUR

		va_end(args);
#if PL_PLATFORM_LINUX || PL_PLATFORM_ANDROID
		raise(SIGINT);
#elif PL_PLATFORM_WINDOWS
		__debugbreak();
#endif
		exit(0);
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
				ImGui::TextWrapped("%s", s_Buffer[index].message.c_str());
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
