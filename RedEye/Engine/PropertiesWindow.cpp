#include "EditorWindow.h"

#include "PropertiesWindow.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "RE_ResourceManager.h"

#include <ImGui/imgui_internal.h>

void PropertiesWindow::Draw(bool secondary)
{
	// draw transform and components
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (RE_RES->GetSelected() != nullptr)
		{
			EA::Thread::Mutex& _r_mutex = RE_RES->GetResourcesMutex();
			EA::Thread::AutoMutex am(_r_mutex);
			const_cast<ResourceContainer*>(RE_RES->At(RE_RES->GetSelected()))->DrawPropieties();
		}
		else if (RE_EDITOR->GetSelected()) RE_SCENE->GetGOPtr(RE_EDITOR->GetSelected())->DrawProperties();

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}