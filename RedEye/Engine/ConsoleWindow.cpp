#include "EditorWindow.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/array.h>

#include "ConsoleWindow.h"

#include "RE_ConsoleLog.h"

#include <ImGui/imgui_internal.h>

ConsoleWindow::ConsoleWindow() : EditorWindow("Console", true)
{
	pos.y = 500.f;
	for (auto& c : categories) c = true;
}

void ConsoleWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_MenuBar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Filter Files"))
			{
				if (ImGui::MenuItem("All")) ChangeFilter(-1);

				eastl::map<eastl::string, unsigned int>::iterator it = callers.begin();
				for (int i = 0; it != callers.end(); i++, it++)
					if (ImGui::MenuItem(it->first.c_str()))
						ChangeFilter(it->second);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Filter Categories"))
			{
				static eastl::array<const char*, 8> category_names = { "Separator", "Global", "Secondary", "Terciary", "Software", "Error" , "Warning", "Solution" };
				for (unsigned int j = 0; j <= category_names.count; j++)
					if (ImGui::MenuItem(category_names[j], categories[j] ? "Hide" : "Show"))
						SwapCategory(j);

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		//Draw console buffer
		if (needs_rewriting) ResetBuffer();
		ImGui::TextEx(console_buffer.begin(), console_buffer.end(), ImGuiTextFlags_NoWidthForLargeClippedText);

		if (scroll_to_bot)
		{
			ImGui::SetScrollHereY(1.f);
			scroll_to_bot = false;
		}
		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

void ConsoleWindow::ResetBuffer()
{
	needs_rewriting = false;
	console_buffer.clear();

	for (const auto& log : logHistory)
	{
		if (categories[log.category] && (file_filter < 0 || log.caller_id == file_filter))
		{
			console_buffer.append(log.data.c_str());

			if (log.count > 1u)
				console_buffer.append((" -> Called: " + eastl::to_string(log.count) + " times\n").c_str());
		}
	}
}

void ConsoleWindow::ChangeFilter(const int new_filter)
{
	if (new_filter != file_filter)
	{
		file_filter = new_filter;
		scroll_to_bot = needs_rewriting = true;
	}
}

void ConsoleWindow::SwapCategory(const unsigned int c)
{
	categories[c] = !categories[c];
	scroll_to_bot = needs_rewriting = true;
}

void ConsoleWindow::AppendLog(unsigned int category, const char* text, const char* file_name)
{
	static unsigned int next_caller_id = 0u;
	unsigned int count = 1u;
	auto caller_id = callers.find(file_name);
	if (caller_id != callers.end())
	{
		for (auto it = logHistory.rbegin(); it != logHistory.rend(); ++it)
		{
			if (it->caller_id == caller_id->second
				&& it->category == category
				&& (eastl::Compare(it->data.c_str(), text, it->data.size()) == 0))
			{
				if (it != logHistory.rbegin())
				{
					count += it->count;
					logHistory.erase(it);
					logHistory.push_back({ caller_id->second, category, count, text });
				}
				else logHistory.rbegin()->count++;

				scroll_to_bot = needs_rewriting = true;
				return;
			}
		}

		logHistory.push_back({ caller_id->second, category, count, text });
	}
	else
	{
		callers.insert(eastl::pair<eastl::string, unsigned int>(file_name, ++next_caller_id));
		logHistory.push_back({ next_caller_id, category, count, text });
	}

	if (!needs_rewriting && categories[category] && (file_filter < 0 || logHistory.back().caller_id == file_filter))
	{
		console_buffer.append(text);
		scroll_to_bot = true;
	}
}