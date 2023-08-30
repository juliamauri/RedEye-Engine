#include "EditorWindow.h"

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

#include <ImGui/imgui_internal.h>

ConfigWindow::ConfigWindow() : EditorWindow("Configuration", true)
{
	pos = { 2000.f, 400.f };
}

void ConfigWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, nullptr, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		DrawOptions();

		DrawModules();

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

void ConfigWindow::DrawOptions() const
{
	if (ImGui::BeginMenu("Options"))
	{
		if (ImGui::MenuItem("Load")) RE_INPUT->Push(RE_EventType::REQUEST_LOAD, App);
		if (ImGui::MenuItem("Save")) RE_INPUT->Push(RE_EventType::REQUEST_SAVE, App);
		ImGui::EndMenu();
	}
}

void ConfigWindow::DrawModules() const
{
	if (ImGui::CollapsingHeader("Time Profiling")) RE_Time::DrawEditorGraphs();
	if (ImGui::CollapsingHeader("Input")) RE_INPUT->DrawEditor();
	if (ImGui::CollapsingHeader("Window")) RE_WINDOW->DrawEditor();
	if (ImGui::CollapsingHeader("Scene")) RE_SCENE->DrawEditor();
	if (ImGui::CollapsingHeader("Physics")) RE_PHYSICS->DrawEditor();
	if (ImGui::CollapsingHeader("Editor")) RE_EDITOR->DrawEditor();
	if (ImGui::CollapsingHeader("Renderer3D")) RE_RENDER->DrawEditor();
	if (ImGui::CollapsingHeader("Audio")) RE_AUDIO->DrawEditor();
	if (ImGui::CollapsingHeader("File System")) RE_FS->DrawEditor();
	if (ImGui::CollapsingHeader("Hardware")) RE_Hardware::DrawEditor();
}
