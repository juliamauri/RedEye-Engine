#include "JR_Window.h"

JR_Window::JR_Window(const char* n) : name(n) { }

JR_Window::~JR_Window() { }

void JR_Window::Draw()
{
	static bool pop;
	if (pop = apply_styles)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(1.00f, 0.43f, 0.00f, 1.00f));
	}

	if (ImGui::Begin(name.c_str(), NULL, w_flags))
	{
		DrawContent();
	}

	ImGui::End();

	if (pop)
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}
}

void JR_Window::PushWindowFlag(ImGuiWindowFlags flag) { w_flags |= flag; }

void JR_Window::PopWindowFlag(ImGuiWindowFlags flag) { w_flags -= flag; }

void JR_Window::SetTitleAlign(ImVec2 align) { apply_styles = true; title_align = align; }

void JR_Window::SetTitleBGColor(ImVec4 color) { apply_styles = true; title_bg_color = color; }