module;

#include <string>
#include <vector>
#include <functional>
#include <imgui.h>

export module CustomWindows;

namespace
{
	struct Window {
		std::string name;
		ImGuiWindowFlags w_flags = ImGuiWindowFlags_None;
		bool apply_styles = false;
		ImVec2 title_align = { 0.0f, 0.5f };
		ImVec4 title_bg_color = { 0.09f, 0.09f, 0.09f, 1.00f };
		std::function<void()> DrawContent;
	};
	static std::vector<Window> _windows;
}

export namespace RE
{
	namespace WindowsManager {
		unsigned int AddWindow(const char* name, std::function<void()> DrawContent)
		{
			_windows.push_back({ name, ImGuiWindowFlags_None, false, ImVec2(0.0f, 0.5f), ImVec4(0.09f, 0.09f, 0.09f, 1.00f), DrawContent });
			return _windows.size() - 1;
		}

		void PushWindowFlag(unsigned int index, ImGuiWindowFlags flag)
		{
			_windows[index].w_flags |= flag;
		}

		void PopWindowFlag(unsigned int index, ImGuiWindowFlags flag)
		{
			_windows[index].w_flags -= flag;
		}

		void SetTitleAlign(unsigned int index, ImVec2 align)
		{
			_windows[index].apply_styles = true;
			_windows[index].title_align = align;
		}

		void SetTitleBGColor(unsigned int index, ImVec4 color)
		{
			_windows[index].apply_styles = true;
			_windows[index].title_bg_color = color;
		}

		void DrawWindows()
		{
			for (auto& window : _windows)
			{
				static bool pop;
				if (pop = window.apply_styles)
				{
					ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, window.title_align);
					ImGui::PushStyleColor(ImGuiCol_TitleBgActive, window.title_bg_color);
				}

				if (ImGui::Begin(window.name.c_str(), NULL, window.w_flags))
				{
					window.DrawContent();
				}

				ImGui::End();

				if (pop)
				{
					ImGui::PopStyleColor();
					ImGui::PopStyleVar();
				}
			}
		}
	}
}