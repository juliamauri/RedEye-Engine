#include "EditorWindows.h"

#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "ModuleRenderer3d.h"
#include "TimeManager.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_Math.h"
#include "OutputLog.h"
#include "ResourceManager.h"
#include "RE_HandleErrors.h"
#include "RE_TextureImporter.h"
#include "RE_InternalResources.h"
#include "RE_CameraManager.h"

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
		std::vector<ResourceContainer*> textures = App->resources->GetResourcesByType(Resource_Type::R_TEXTURE);
		if (!textures.empty())
		{
			std::vector<ResourceContainer*>::iterator it = textures.begin();
			for (it; it != textures.end(); ++it) {

				if (ImGui::TreeNode((*it)->GetName()))
				{
					Texture2D* texture = (Texture2D*)(*it);
					int widht, height;
					texture->GetWithHeight(&widht, &height);
					ImGui::Text("GL ID: %u", texture->GetID());
					ImGui::Text("Widht: %u", widht);
					ImGui::Text("Height: %u", height);
					texture->DrawTextureImGui();
					ImGui::TreePop();
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
		{
			// check main camera
			if (RE_CameraManager::HasMainCamera())
				App->ScenePlay();
			// else { report problems }
		}
		ImGui::SameLine();
		if (ImGui::Button("Pause")) App->ScenePause();
		ImGui::SameLine();
		if (ImGui::Button("Stop")) App->SceneStop();
		ImGui::SameLine();
		ImGui::Text("%.2f", seconds);

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

SelectFile::SelectFile(const char * name, bool start_active) : EditorWindow(name, start_active) {}

void SelectFile::Start(const char* windowName, const char * path, std::string* forFill, bool selectFolder)
{
	if (active == false) {
		active = true;
		this->windowName = windowName;
		this->path = path;
		toFill = forFill;
		selected.clear();
		selectingFolder = selectFolder;

		rc = PHYSFS_enumerateFiles(path);
	}
}

void SelectFile::SelectTexture()
{
	textures = App->resources->GetResourcesByType(Resource_Type::R_TEXTURE);
}

ResourceContainer * SelectFile::GetSelectedTexture()
{
	ResourceContainer* ret = selectedTexture;
	Clear();
	return ret;
}

void SelectFile::Clear()
{
	active = false;
	textures.clear();
	selectedTexture = nullptr;
	if(rc) PHYSFS_freeList(rc);
	rc = nullptr;
	selectedPointer = nullptr;
	selected.clear();
	windowName.clear();
	toFill = nullptr;
}

void SelectFile::Draw(bool secondary)
{
	if(ImGui::Begin((windowName.empty()) ? name : windowName.c_str(), 0))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (rc != nullptr)
		{
			char **i;
			for (i = rc; *i != NULL; i++)
			{
				bool popColor = false;
				if (selectedPointer != nullptr && *selectedPointer == *i) {
					popColor = true;
					ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.5f, 0.0f, 0.0f, 1.0f });
					ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, { 1.0f, 0.0f, 0.0f, 1.0f });
				}
				if (ImGui::Button(*i))
				{
					selectedPointer = i;
					selected = path;
					selected += *i;
				}

				if (popColor) {
					ImGui::PopStyleColor(2);
				}
			}
		}
		else if (!textures.empty()) {

			for (auto textureResource : textures) {
				bool popColor = false;
				if (selectedTexture != nullptr && selectedTexture == textureResource) {
					popColor = true;
					ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.5f, 0.0f, 0.0f, 1.0f });
					ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, { 1.0f, 0.0f, 0.0f, 1.0f });
				}
				if (ImGui::Button(textureResource->GetName()))
				{
					selectedTexture = textureResource;
				}

				if (popColor) {
					ImGui::PopStyleColor(2);
				}
			}

		}

		bool popStyle = false;
		if (!secondary && selected.empty()) {
			popStyle = true;
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::Button("Apply")) {
			if (rc != nullptr) {
				if (selectingFolder)  selected += "/";
				*toFill = selected;
				Clear();
			}
			else if (!textures.empty()) {
				active = false;
			}
		}

		if (popStyle && !secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

void SelectFile::SendSelected()
{
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
		
		ImGui::SameLine();

		if (ImGui::Button("Create Prefab"))
		{
			if (App->scene->GetSelected() != nullptr)
			{
				App->resources->Reference(new RE_Prefab(App->scene->GetSelected()));
				prefabs = App->resources->GetResourcesByType(Resource_Type::R_PREFAB);
			}
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
				if(ImGui::Button((selected == currentPrefab) ? std::string(std::string("Selected -> ") + std::string(currentPrefab->GetName())).c_str() : currentPrefab->GetName()))
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
		if (fromHandleError)
		{
			// Error
			ImGui::TextColored(ImVec4(255.f, 0.f, 0.f, 1.f), !App->handlerrors->AnyErrorHandled() ?
				"No errors" :
				App->handlerrors->GetErrors(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
			
			ImGui::Separator();

			// Solution
			ImGui::TextColored(ImVec4(0.f, 255.f, 0.f, 1.f), !App->handlerrors->AnyErrorHandled() ?
				"No solutions" :
				App->handlerrors->GetSolutions(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);

			ImGui::Separator();

			// Accept Button
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

			// Logs
			if (ImGui::TreeNode("Show All Logs"))
			{
				ImGui::TextEx(App->handlerrors->GetLogs(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
				ImGui::TreePop();
			}

			// Warnings
			if (ImGui::TreeNode("Show Warnings"))
			{
				ImGui::TextEx(!App->handlerrors->AnyWarningHandled() ?
					"No warnings" :
					App->handlerrors->GetWarnings(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
				ImGui::TreePop();
			}
		}
		else if (ImGui::Button(btnText.c_str()))
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

SkyBoxWindow::SkyBoxWindow(const char * name, bool start_active) : EditorWindow(name, start_active) 
{
	skyboxesPath = "Assets/Skyboxes/";
	skyboxPathSelected = skyboxesPath + "default/";
}

void SkyBoxWindow::SetSkyBoxPath(const char * path)
{
	skyboxPathSelected = path;
}

void SkyBoxWindow::SetTextures(std::string texturesname[6])
{
	for (uint i = 0; i < 6; i++) {
		texturesPath[i] = texturesname[i];
	}
}

void SkyBoxWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::SliderFloat("SkyBox size", &skyBoxSize, 0.0f, 10000.0f))
			applySize = true;

		ImGui::Text("Skybox textures folder selected:\n%s", skyboxPathSelected.c_str());
		if (ImGui::Button("Select Skybox Folder")) {
			std::string s = "Select folder";
			App->editor->GetSelectWindow()->Start(s.c_str(), skyboxesPath.c_str(), &skyboxPathSelected, true);
		}

		for (uint i = 0; i < 6; i++) {

			if (ImGui::TreeNodeEx(texturesname[i], ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) {


				if (!texturesPath[i].empty())
					ImGui::Text("Path: %s", texturesPath[i].c_str());
				else
					ImGui::Text("%s texture don't exists", texturesname[i]);

				if (ImGui::Button("Select texture")) {
					std::string s = "Select ";
					s += texturesname[i];
					s += " file";
					texturesPath[i].clear();
					App->editor->GetSelectWindow()->Start(s.c_str(), skyboxPathSelected.c_str(), &texturesPath[i]);
					applyTextures = true;
				}
				ImGui::TreePop();
			}
		}

		if (ImGui::Button("Apply Changes")) {

			if (applySize) App->internalResources->CreateSkyBoxCube(skyBoxSize);

			if (applyTextures) {
				const char* texs[6] = { texturesPath[0].c_str(), texturesPath[1].c_str(), texturesPath[2].c_str(), texturesPath[3].c_str(),
				texturesPath[4].c_str(), texturesPath[5].c_str() };
				App->internalResources->ChangeSkyBoxTexturesID(App->textures->LoadSkyBoxTextures(texs));
			}

			applySize = false;
			applyTextures = false;
		}

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}
