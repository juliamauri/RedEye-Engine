#include "EditorWindow.h"

#include "PropertiesWindow.h"

#include "Application.h"
#include "RE_ResourceManager.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"

#include <ImGui/imgui_internal.h>

void PropertiesWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		auto selected_resource = RE_RES->GetSelected();
		if (selected_resource) RE_RES->At(selected_resource)->DrawPropieties();
		else 
		{
			auto selected_go = RE_EDITOR->GetSelected();
			if (selected_go) RE_SCENE->GetGOPtr(selected_go)->DrawProperties();
		}

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}