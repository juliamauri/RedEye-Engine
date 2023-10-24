#include "EditorWindow.h"

#include "HierarchyWindow.h"

#include "Application.h"
#include "ModuleEditor.h"

#include <ImGui/imgui_internal.h>

void HierarchyWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, nullptr, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		RE_EDITOR->DrawHeriarchy();

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}