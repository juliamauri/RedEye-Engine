#include "EditorWindows.h"

#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "TimeManager.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_Math.h"
#include "OutputLog.h"
#include "ResourceManager.h"
#include "ModelHandleErrors.h"

#include "RE_Prefab.h"

#include "PhysFS\include\physfs.h"

#include <map>

#include "ImGui/imgui_internal.h"

EditorWindow::EditorWindow(const char* name, bool start_enabled)
	: name(name), active(start_enabled), lock_pos(false)
{
	/*default_flags = default_flags = 
		  ImGuiWindowFlags_NoFocusOnAppearing
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove;*/
}

EditorWindow::~EditorWindow()
{}

void EditorWindow::DrawWindow(bool secondary)
{
	if (lock_pos)
	{
		ImGui::SetNextWindowPos(pos);
		ImGui::SetWindowSize(size);
	}

	Draw(secondary);
}

void EditorWindow::SwitchActive()
{
	active = !active;
}

const char * EditorWindow::Name() const { return name; }

bool EditorWindow::IsActive() const { return active; }


///////   Console Window   ////////////////////////////////////////////

ConsoleWindow::ConsoleWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{
	pos.y = 500.f;
}

void ConsoleWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_MenuBar))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Filter Files"))
			{
				if (ImGui::MenuItem("All")) ChangeFilter(-1);

				std::map<std::string, unsigned int>::iterator it = App->log->callers.begin();
				for (int i = 0; it != App->log->callers.end(); i++, it++)
				{
					if (ImGui::MenuItem(it->first.c_str())) ChangeFilter(it->second);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Filter Categories"))
			{
				for (unsigned int j = 0; j < L_TOTAL_CATEGORIES; j++)
					if (ImGui::MenuItem(category_names[j], categories[j] ? "Hide" : "Show"))
						SwapCategory(j);

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		
		//Draw console buffer //TODO print parcial buffer, play with cursors
		ImGui::TextEx(console_buffer.begin(), console_buffer.end(), ImGuiTextFlags_NoWidthForLargeClippedText);

		if (scroll_to_bot)
		{
			ImGui::SetScrollHere(1.f);
			scroll_to_bot = false;
		}
		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

	}
	ImGui::End();
}

void ConsoleWindow::ChangeFilter(const int new_filter)
{
	if (new_filter != file_filter)
	{
		file_filter = new_filter;
		console_buffer.clear();
		scroll_to_bot = true;

		std::list<RE_Log>::iterator it = App->log->logHistory.begin();

		if (file_filter < 0)
		{
			for (; it != App->log->logHistory.end(); it++)
				if (categories[it->category])
					console_buffer.append(it->data.c_str());
		}
		else
		{
			for (; it != App->log->logHistory.end(); it++)
				if (it->caller_id == file_filter && categories[it->category])
					console_buffer.append(it->data.c_str());
		}
	}
}

void ConsoleWindow::SwapCategory(const unsigned int c)
{
	categories[c] = !categories[c];
	console_buffer.clear();
	scroll_to_bot = true;

	std::list<RE_Log>::iterator it = App->log->logHistory.begin();

	if (file_filter < 0)
	{
		for (; it != App->log->logHistory.end(); it++)
			if (categories[it->category])
				console_buffer.append(it->data.c_str());
	}
	else
	{
		for (; it != App->log->logHistory.end(); it++)
			if (it->caller_id == file_filter && categories[it->category])
				console_buffer.append(it->data.c_str());
	}
}


///////   Configuration Window   ////////////////////////////////////////////

ConfigWindow::ConfigWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{
	changed_config = false;
	pos.x = 2000.f;
	pos.y = 400.f;
}

void ConfigWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		App->DrawEditor();

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

	}
	ImGui::End();
}


///////   Heriarchy Window   ////////////////////////////////////////////

HeriarchyWindow::HeriarchyWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{}

void HeriarchyWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if(App->scene)
			App->scene->DrawHeriarchy();

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}


///////   Properties Window   ////////////////////////////////////////////

PropertiesWindow::PropertiesWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{}

void PropertiesWindow::Draw(bool secondary)
{
	// draw transform and components
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (App->scene) App->scene->DrawFocusedProperties();

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}


///////   About Window   ////////////////////////////////////////////

SoftwareInfo::SoftwareInfo(const char * name, const char * v, const char * w) :
	name(name)
{
	if (v != nullptr) version = v;
	if (w != nullptr) website = w;
}

AboutWindow::AboutWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{}

void AboutWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Engine name: %s", App->GetName());
		ImGui::Text("Organization: %s", App->GetOrganization());
		ImGui::Text("License: GNU General Public License v3.0");

		ImGui::Separator();
		ImGui::Text("%s is a 3D Game Engine Sofware for academic purposes.", App->GetName());

		ImGui::Separator();
		ImGui::Text("Authors:");
		ImGui::Text("Juli Mauri Costa");
		ImGui::SameLine();
		if (ImGui::Button("Visit Juli's Github Profile"))
			BROWSER("https://github.com/juliamauri");
		ImGui::Text("Ruben Sardon Roldan");
		ImGui::SameLine();
		if (ImGui::Button("Visit Ruben's Github Profile"))
			BROWSER("https://github.com/cumus");

		ImGui::Separator();
		if (ImGui::CollapsingHeader("3rd Party Software:"))
		{
			std::list<SoftwareInfo>::iterator it = sw_info.begin();
			for (; it != sw_info.end(); ++it)
			{
				if (!it->name.empty())
				{
					if (!it->version.empty())
						ImGui::BulletText("%s: v%s ", it->name.c_str(), it->version.c_str());
					else
						ImGui::BulletText("%s ", it->name.c_str());

					if (!it->website.empty())
					{
						std::string button_name = "Open ";
						button_name += it->name;
						button_name += " Website";
						ImGui::SameLine();
						if (ImGui::Button(button_name.c_str()))
							BROWSER(it->website.c_str());
					}
				}
			}

			/* Missing Library Initialicers
			SDL Mixer v2.0 https://www.libsdl.org/projects/SDL_mixer/
			SDL TTF v2.0 https://www.libsdl.org/projects/SDL_ttf/
			DevIL v1.8.0 http://openil.sourceforge.net/
			Open Asset Import Library http://www.assimp.org/ */
		}

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();

}


///////   Random Tool   ////////////////////////////////////////////

RandomTest::RandomTest(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{}

void RandomTest::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Random Integer");
		ImGui::SliderInt("Min Integer", &minInt, 0, maxInt);
		ImGui::SliderInt("Max Integer", &maxInt, minInt, 100);

		if (ImGui::Button("Generate Int"))
			resultInt = App->math->RandomInt(minInt, maxInt);

		ImGui::SameLine();
		ImGui::Text("Random Integer: %u", resultInt);

		ImGui::Separator();

		ImGui::Text("Random Float");
		ImGui::SliderFloat("Min Float", &minF, -100.f, maxF, "%.1f");
		ImGui::SliderFloat("Max Float", &maxF, minF, 100.f, "%.1f");

		if (ImGui::Button("Generate Float"))
			resultF = App->math->RandomF(minF, maxF);

		ImGui::SameLine();
		ImGui::Text("Random Float: %.2f", resultF);

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}


///////   Texture Manager  ////////////////////////////////////////////
TexturesWindow::TexturesWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}

void TexturesWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		/*
		std::vector<Texture2D*>* textures = App->textures->GetTextures();
		if (textures->size() != 0)
		{
			std::vector<Texture2D*>::iterator it = textures->begin();
			for (it; it != textures->end(); ++it) {

				if (ImGui::TreeNode((*it)->GetName()))
				{
					int widht, height;
					(*it)->GetWithHeight(&widht, &height);
					ImGui::Text("GL ID: %u", (*it)->GetID());
					ImGui::Text("Widht: %u", widht);
					ImGui::Text("Height: %u", height);
					(*it)->DrawTextureImGui();
					ImGui::TreePop();
				}
			}
		}
		*/
		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

EditorSettingsWindow::EditorSettingsWindow(const char * name, bool start_active) : EditorWindow(name, start_active) {}

void EditorSettingsWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		RE_CompCamera* cam = App->editor->GetCamera();
		cam->DrawProperties();
		cam->GetTransform()->DrawProperties();

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

PlayPauseWindow::PlayPauseWindow(const char * name, bool start_active) : EditorWindow(name, start_active) {}

void PlayPauseWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		float seconds = App->time->GetGameTimer();
		if (ImGui::Button(App->GetState() == GS_PLAY ? "Restart" : "Play"))
			App->ScenePlay();
		ImGui::SameLine();
		if (ImGui::Button("Pause"))
			App->ScenePause();
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
			App->SceneStop();
		ImGui::SameLine();
		ImGui::Text("%f", seconds);

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

SelectFile::SelectFile(const char * name, bool start_active) : EditorWindow(name, start_active) {}

void SelectFile::Start(const char * path)
{
	SwitchActive();
	this->path = path;
	selected.clear();

	rc = PHYSFS_enumerateFiles(path);
}

std::string SelectFile::IsSelected()
{
	if (!selected.empty())
	{
		SwitchActive();
		PHYSFS_freeList(rc);
		return selected;
	}
	else
		return "";
}

void SelectFile::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (selected.empty())
		{
			char **i;
			for (i = rc; *i != NULL; i++)
			{
				if (ImGui::Button(*i))
				{
					selected = path;
					selected += *i;
					break;
				}
			}
		}

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

PrefabsPanel::PrefabsPanel(const char * name, bool start_active) : EditorWindow(name, start_active) {}

void PrefabsPanel::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::Button("Reload Prefabs"))
			prefabs = App->resources->GetResourcesByType(Resource_Type::R_PREFAB);
		
		ImGui::SameLine();

		if (ImGui::Button("Create Prefab"))
		{
			if (App->scene->GetSelected() != nullptr)
				App->resources->Reference(new RE_Prefab(App->scene->GetSelected()));
		}

		if (ImGui::Button("Clone selected to Scene"))
		{
			if (selected != nullptr)
				App->scene->AddGoToRoot(((RE_Prefab*)selected)->GetRoot());
		}

		ImGui::NewLine();

		if (!prefabs.empty())
		{
			ImGui::Text("Current Prefabs");

			for (ResourceContainer* currentPrefab : prefabs)
			{
				if(ImGui::Button(currentPrefab->GetName()))
					selected = currentPrefab;
			}

		}
		else
			ImGui::Text("Empty");

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

PopUpWindow::PopUpWindow(const char * name, bool start_active) : EditorWindow(name, start_active) {}

void PopUpWindow::PopUp(const char * _btnText, const char* title, bool _disableAllWindows)
{
	btnText = _btnText;
	titleText = title;
	if (disableAllWindows = _disableAllWindows) App->editor->PopUpFocus(disableAllWindows);
	active = true;
}

void PopUpWindow::PopUpError()
{
	fromHandleError = true;
	PopUp("Accept", "Error", true);
}

void PopUpWindow::Draw(bool secondary)
{
	if(ImGui::Begin(titleText.c_str(), 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (fromHandleError) {
			if (ImGui::CollapsingHeader("All logs", 0, ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar)) {
				ImGui::TextEx(App->handlerrors->GetLogs(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
			}

			static bool anyWarning = App->handlerrors->AnyWarningHandled();
			if (ImGui::CollapsingHeader("Warnings", 0, ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar)) {
				ImGui::TextEx((!anyWarning) ? "No warnings" : App->handlerrors->GetWarnings(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
			}

			static bool anyError = App->handlerrors->AnyErrorHandled();
			if (ImGui::CollapsingHeader("Errors", 0, ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse)) {
				ImGui::TextEx((!anyError) ? "No errors" :  App->handlerrors->GetErrors(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
			}

			static bool anySolution = App->handlerrors->AnyErrorHandled();
			if (ImGui::CollapsingHeader("Solutions", 0, ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse)) {
				ImGui::TextEx((!anySolution) ? "No solutions" : App->handlerrors->GetSolutions(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
			}
		}

		if (ImGui::Button(btnText.c_str()))
		{
			active = false;
			disableAllWindows = false;
			App->editor->PopUpFocus(disableAllWindows);
			if (fromHandleError) {
				App->handlerrors->ClearAll();
				fromHandleError = false;
			}
		}
	}
	ImGui::End();
}
