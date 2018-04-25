#include "imgui_impl/Log.h"
#include <stdexcept>
#include "GLFW/glfw3.h"
#include "Application.h"

ImGuiTextBuffer* Log::m_Buffer = nullptr;
bool Log::m_ScrollToBottom = false;

void Log::Clear()
{
	m_Buffer->clear();
}

void Log::Info(const char* fmt, ...) IM_PRINTFARGS(2)
{
	if (!m_Buffer)
		m_Buffer = new ImGuiTextBuffer();

	va_list args;
	va_start(args, fmt);
	m_Buffer->appendv(fmt, args);
	m_Buffer->appendv("\n", args);
	va_end(args);
	m_ScrollToBottom = true;
}

void Log::Fatal(const char* fmt, ...) IM_PRINTFARGS(2)
{
	if (!m_Buffer)
		m_Buffer = new ImGuiTextBuffer();

	va_list args;
	va_start(args, fmt);
	m_Buffer->appendv(fmt, args);
	m_Buffer->appendv("\n", args);
	va_end(args);
	m_ScrollToBottom = true;
	assert(false);
}

void Log::Draw(const char* title)
{
	if (!m_Buffer)
		m_Buffer = new ImGuiTextBuffer();

	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiSetCond_FirstUseEver);
	ImGui::Begin(title);
	int scrollY = -1;

	if (ImGui::Button("Clear")) Clear();
	ImGui::SameLine();
	bool copy = ImGui::Button("Copy");
	ImGui::Separator();
	ImGui::BeginChild("scrolling");
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
	if (copy) ImGui::LogToClipboard();
	ImGui::TextUnformatted(m_Buffer->begin());
	if (m_ScrollToBottom)
		ImGui::SetScrollHere(1.0f);

	m_ScrollToBottom = false;
	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::End();
}
