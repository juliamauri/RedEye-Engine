#include "EditorWindow.h"

#include "PlayPauseWindow.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "RE_Time.h"
#include "RE_CameraManager.h"
#include "SceneEditorWindow.h"

#include <SDL2/SDL_scancode.h>
#include <ImGui/imgui_internal.h>
#include <ImGuiWidgets/ImGuizmo/ImGuizmo.h>

void PlayPauseWindow::Draw(bool secondary)
{
	if (!ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		ImGui::End();
		return;
	}

	if (secondary)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	if (RE_CameraManager::HasMainCamera())
	{
		switch (RE_Time::DrawEditorControls())
		{
		case RE_Time::GameState::PLAY:  RE_INPUT->Push(RE_EventType::PLAY, App); break;
		case RE_Time::GameState::PAUSE: RE_INPUT->Push(RE_EventType::PAUSE, App); break;
		case RE_Time::GameState::STOP:  RE_INPUT->Push(RE_EventType::STOP, App); break;
		case RE_Time::GameState::TICK:  RE_INPUT->Push(RE_EventType::TICK, App); break;
		default: break;
		}
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

	if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_Q)->Down) { o = ImGuizmo::OPERATION::TRANSLATE; changed = true; }
	if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_W)->Down) { o = ImGuizmo::OPERATION::ROTATE;    changed = true; }
	if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_E)->Down) { o = ImGuizmo::OPERATION::SCALE;	    changed = true; }

	if (!colored && o == ImGuizmo::OPERATION::TRANSLATE)
	{
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 1.0f });
		colored = true;
	}

	if (ImGui::Button("Translate"))
	{
		o = ImGuizmo::OPERATION::TRANSLATE;
		changed = true;
	}

	if (colored)
	{
		ImGui::PopStyleColor();
		colored = false;
	}

	if (!colored && !changed && o == ImGuizmo::OPERATION::ROTATE)
	{
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 1.0f });
		colored = true;
	}

	ImGui::SameLine();
	if (ImGui::Button("Rotate"))
	{
		o = ImGuizmo::OPERATION::ROTATE;
		changed = true;
	}

	if (colored)
	{
		ImGui::PopStyleColor();
		colored = false;
	}

	if (!colored && !changed && o == ImGuizmo::OPERATION::SCALE)
	{
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 1.0f });
		colored = true;
	}

	ImGui::SameLine();
	if (ImGui::Button("Scale"))
	{
		o = ImGuizmo::OPERATION::SCALE;
		changed = true;
	}

	if (colored)
	{
		ImGui::PopStyleColor();
		colored = false;
	}

	if (changed)
	{
		RE_EDITOR->GetSceneEditor()->SetOperation(o);
		changed = false;
	}


	ImGui::SameLine();
	static ImGuizmo::MODE m = RE_EDITOR->GetSceneEditor()->GetMode();
	if (ImGui::Button((m == ImGuizmo::MODE::LOCAL) ? "Local Transformation" : "Global Transformation"))
	{
		switch (m)
		{
		case ImGuizmo::MODE::LOCAL: m = ImGuizmo::MODE::WORLD; break;
		case ImGuizmo::MODE::WORLD: m = ImGuizmo::MODE::LOCAL; break;
		default: break;
		}
		RE_EDITOR->GetSceneEditor()->SetMode(m);
	}

	if (secondary)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	ImGui::End();
}