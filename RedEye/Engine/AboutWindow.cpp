#include "AboutWindow.h"

#include "RE_EngineInfo.h"
#include "Application.h"

#include <ImGui/imgui_internal.h>
#include <EAStdC/EASprintf.h>

void AboutWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, nullptr, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Engine name: %s", ENGINE_NAME);
		ImGui::Text("Version: %s", ENGINE_VERSION);
		ImGui::Text("Organization: %s", ENGINE_ORGANIZATION);
		ImGui::Text("License: %s", ENGINE_LICENSE);

		ImGui::Separator();
		ImGui::Text(ENGINE_DESCRIPTION);

		ImGui::Separator();
		ImGui::Text("Authors:");
		ImGui::Text(ENGINE_AUTHOR_1);
		ImGui::SameLine();
		if (ImGui::Button("Visit Github Profile")) BROWSER("https://github.com/juliamauri");
		ImGui::Text(ENGINE_AUTHOR_2);
		ImGui::SameLine();
		if (ImGui::Button("Visit Github Profile")) BROWSER("https://github.com/cumus");

		DrawThirdParties();

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

void AboutWindow::DrawThirdParties() const
{
	ImGui::Separator();
	if (ImGui::CollapsingHeader("3rd Party Software:"))
	{
		for (auto& software : sw_info)
		{
			if (software.version != nullptr) ImGui::BulletText("%s: v%s ", software.name, software.version);
			else ImGui::BulletText("%s ", software.name);

			if (software.website != nullptr)
			{
				ImGui::SameLine();
				static eastl::string tmp;
				tmp.clear();
				tmp.reserve(124);

				EA::StdC::Snprintf(tmp.data(), 128, "Open %s Website", software.name);
				if (ImGui::Button(tmp.c_str())) BROWSER(software.website);
			}
		}
	}
}
