#include "WwiseWindow.h"

#include "Application.h"
#include "ModuleAudio.h"

#include "ImGui/imgui_internal.h"
#include "ImGuiImplementations/imgui_stdlib.h"
#include <EASTL/string.h>

void WwiseWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		static bool usingRTPC = false;
		static bool usingState = false;
		static bool usingSwitch = false;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Send RTPC value"))
			{
				usingRTPC = true;
				usingState = usingSwitch = false;
			}
			if (ImGui::MenuItem("Send State value"))
			{
				usingState = true;
				usingRTPC = usingSwitch = false;
			}
			if (ImGui::MenuItem("Send Switch value"))
			{
				usingSwitch = true;
				usingState = usingRTPC = false;
			}
			ImGui::EndMenuBar();
		}

		if (usingRTPC || usingState || usingSwitch)
		{
			if (usingRTPC)
			{
				static eastl::string name = "RTPC_Name";
				static float value = 0;
				ImGui::InputText("Insert RTPC name", &name);
				ImGui::InputFloat("Insert RTPC value", &value);
				if (ImGui::Button("Send RTPC Value")) ModuleAudio::SendRTPC(name.c_str(), value);
			}
			else if (usingState)
			{
				static eastl::string group = "StateGroup_Name";
				static eastl::string state = "State_Name";
				ImGui::InputText("Insert State group name", &group);
				ImGui::InputText("Insert State name", &state);
				if (ImGui::Button("Send State Value")) ModuleAudio::SendState(group.c_str(), state.c_str());
			}
			else if (usingSwitch)
			{
				static eastl::string switchname = "Switch_Name";
				static eastl::string switchstate = "SwitchState_Name";
				ImGui::InputText("Insert Switch name", &switchname);
				ImGui::InputText("Insert Switch state name", &switchstate);
				if (ImGui::Button("Send Switch Value")) ModuleAudio::SendSwitch(switchname.c_str(), switchstate.c_str());
			}

			ImGui::Separator();
		}

		RE_AUDIO->DrawWwiseElementsDetected();

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
}
