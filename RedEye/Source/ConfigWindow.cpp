#include "ConfigWindow.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "ModulePhysics.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "ModuleAudio.h"
#include "RE_Time.h"
#include "RE_Hardware.h"
#include "RE_FileSystem.h"

#include "ImGui/imgui_internal.h"

ConfigWindow::ConfigWindow(const char* name, bool start_active) : EditorWindow(name, start_active)
{
	changed_config = false;
	pos.x = 2000.f;
	pos.y = 400.f;
}

ConfigWindow::~ConfigWindow() {}

void ConfigWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::BeginMenu("Options"))
		{
			if (ImGui::MenuItem("Load")) RE_INPUT->Push(RE_EventType::REQUEST_LOAD, App);
			if (ImGui::MenuItem("Save")) RE_INPUT->Push(RE_EventType::REQUEST_SAVE, App);
			ImGui::EndMenu();
		}

		if (ImGui::CollapsingHeader("Time Profiling")) RE_TIME->DrawEditorGraphs();

		RE_INPUT->DrawEditor();
		RE_WINDOW->DrawEditor();
		RE_SCENE->DrawEditor();
		RE_PHYSICS->DrawEditor();
		RE_EDITOR->DrawEditor();
		RE_RENDER->DrawEditor();
		RE_AUDIO->DrawEditor();

		if (ImGui::CollapsingHeader("File System")) RE_FS->DrawEditor();
		if (ImGui::CollapsingHeader("Hardware")) RE_HARDWARE->DrawEditor();

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}