#pragma once
#include "imgui/imgui.h"

class Log
{
public:

	static void Clear();
	static void Info(const char* fmt, ...) IM_FMTARGS(2);
	static void Fatal(const char* fmt, ...) IM_FMTARGS(2);

	static void Draw(const char* title);

private:
	static ImGuiTextBuffer* m_Buffer;
	static bool m_ScrollToBottom;
};