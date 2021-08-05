#include "PlayPauseWindow.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "RE_Time.h"
#include "RE_CameraManager.h"
#include "SceneEditorWindow.h"

#include "ImGuiWidgets/ImGuizmo/ImGuizmo.h"


#include "SDL2/include/SDL_scancode.h"
#include "ImGui/imgui_internal.h"

void PlayPauseWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (RE_CameraManager::HasMainCamera())
		{
			switch (RE_TIME->DrawEditorControls()) {
			case GS_PLAY:  RE_INPUT->Push(RE_EventType::PLAY, App); break;
			case GS_PAUSE: RE_INPUT->Push(RE_EventType::PAUSE, App); break;
			case GS_STOP:  RE_INPUT->Push(RE_EventType::STOP, App); break;
			case GS_TICK:  RE_INPUT->Push(RE_EventType::TICK, App); break;
			default: break; }
		}
		else ImGui::Text("Missing Main Camera");

		ImGui::SameLine();
		ImGui::Checkbox("Draw Gizmos", &RE_EDITOR->debug_drawing);

		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_::ImGuiSeparatorFlags_Vertical);

		static ImGuizmo::OPERATION o = RE_EDITOR->GetSceneEditor()->GetOperation();
		static bool changed = false;
		static bool colored = false;
		ImGui::SameLine();

		if (RE_INPUT->GetKey(SDL_SCANCODE_Q) == KEY_STATE::KEY_DOWN) { o = ImGuizmo::OPERATION::TRANSLATE; changed = true; }
		if (RE_INPUT->GetKey(SDL_SCANCODE_W) == KEY_STATE::KEY_DOWN) { o = ImGuizmo::OPERATION::ROTATE;    changed = true; }
		if (RE_INPUT->GetKey(SDL_SCANCODE_E) == KEY_STATE::KEY_DOWN) { o = ImGuizmo::OPERATION::SCALE;	    changed = true; }

		if (!colored && o == ImGuizmo::OPERATION::TRANSLATE) {
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 1.0f });
			colored = true;
		}

		if (ImGui::Button("Translate")) {
			o = ImGuizmo::OPERATION::TRANSLATE;
			changed = true;
		}

		if (colored) {
			ImGui::PopStyleColor();
			colored = false;
		}

		if (!colored && !changed && o == ImGuizmo::OPERATION::ROTATE) {
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 1.0f });
			colored = true;
		}

		ImGui::SameLine();
		if (ImGui::Button("Rotate")) {
			o = ImGuizmo::OPERATION::ROTATE;
			changed = true;
		}

		if (colored) {
			ImGui::PopStyleColor();
			colored = false;
		}

		if (!colored && !changed && o == ImGuizmo::OPERATION::SCALE) {
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 1.0f });
			colored = true;
		}

		ImGui::SameLine();
		if (ImGui::Button("Scale")) {
			o = ImGuizmo::OPERATION::SCALE;
			changed = true;
		}

		if (colored) {
			ImGui::PopStyleColor();
			colored = false;
		}

		if (changed) {
			RE_EDITOR->GetSceneEditor()->SetOperation(o);
			changed = false;
		}


		ImGui::SameLine();
		static ImGuizmo::MODE m = RE_EDITOR->GetSceneEditor()->GetMode();
		if (ImGui::Button((m == ImGuizmo::MODE::LOCAL) ? "Local Transformation" : "Global Transformation"))
		{
			switch (m) {
			case ImGuizmo::MODE::LOCAL: m = ImGuizmo::MODE::WORLD; break;
			case ImGuizmo::MODE::WORLD: m = ImGuizmo::MODE::LOCAL; break;
			}
			RE_EDITOR->GetSceneEditor()->SetMode(m);
		}

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}