#include "EditorWindows.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "ModuleRenderer3d.h"
#include "TimeManager.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_Math.h"
#include "OutputLog.h"
#include "RE_ResourceManager.h"
#include "RE_ShaderImporter.h"
#include "RE_HandleErrors.h"
#include "RE_TextureImporter.h"
#include "RE_InternalResources.h"
#include "RE_CameraManager.h"
#include "RE_Prefab.h"
#include "RE_Texture.h"
#include "RE_Material.h"
#include "RE_Shader.h"

#include "PhysFS\include\physfs.h"

#include "ImGui/misc/cpp/imgui_stdlib.h"
#include "ImGui/imgui_internal.h"

#include <map>

std::queue<Event> Event::events_queue;

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

ConsoleWindow::~ConsoleWindow()
{
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

ConfigWindow::~ConfigWindow()
{
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

HeriarchyWindow::~HeriarchyWindow()
{
}

void HeriarchyWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if(App->scene)
			App->editor->DrawHeriarchy();

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

PropertiesWindow::~PropertiesWindow()
{
}

void PropertiesWindow::Draw(bool secondary)
{
	// draw transform and components
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (App->resources && App->resources->GetSelected() != nullptr)
			App->resources->At(App->resources->GetSelected())->DrawPropieties();
		else if (App->editor && App->editor->GetSelected() != nullptr)
			App->editor->GetSelected()->DrawProperties();

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

AboutWindow::~AboutWindow()
{
}

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

RandomTest::~RandomTest()
{
}

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

TexturesWindow::~TexturesWindow()
{
}

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
					RE_Texture* texture = (RE_Texture*)(*it);
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

PlayPauseWindow::~PlayPauseWindow()
{
}

void PlayPauseWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		float seconds = App->time->GetGameTimer();
		const GameState state = App->GetState();

		// PLAY / RESTART
		if (ImGui::Button(state == GS_PLAY ? "Restart" : " Play  "))
		{
			// check main camera
			if (RE_CameraManager::HasMainCamera())
				Event::Push(PLAY, App);
			// else { report problems }
		}

		ImGui::SameLine();

		// PAUSE / TICK
		if (ImGui::Button(state != GS_PLAY ? "Tick " : "Pause"))
			Event::Push(state != GS_PLAY ? TICK : PAUSE, App);

		ImGui::SameLine();

		// STOP
		if (ImGui::Button("Stop") && state != GS_STOP)
			Event::Push(STOP, App);

		ImGui::SameLine();

		ImGui::Text("%.2f", seconds);

		ImGui::SameLine();
		ImGui::Checkbox("Draw Gizmos", &App->editor->debug_drawing);

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

SelectFile::SelectFile(const char * name, bool start_active) : EditorWindow(name, start_active) {}

SelectFile::~SelectFile()
{
}

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

PrefabsPanel::~PrefabsPanel()
{
}

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
			if (App->editor->GetSelected() != nullptr)
			{
				RE_Prefab* newPrefab = new RE_Prefab();
				RE_GameObject* selected = App->editor->GetSelected();
				newPrefab->SetName(selected->GetName());
				newPrefab->Save(selected);
				newPrefab->SaveMeta();
				App->resources->Reference(newPrefab);
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

PopUpWindow::~PopUpWindow()
{
}

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

AssetsWindow::AssetsWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}

AssetsWindow::~AssetsWindow()
{
}

const char* AssetsWindow::GetCurrentDirPath() const
{
	return currentPath;
}

void AssetsWindow::SelectUndefined(std::string* toFill)
{
	selectingUndefFile = toFill;
}

void AssetsWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		static RE_FileSystem::RE_Directory* currentDir = App->fs->GetRootDirectory();
		RE_FileSystem::RE_Directory* toChange = nullptr;

		if (currentDir->parent == nullptr)
		{
			ImGui::Text("%s Folder", currentDir->name.c_str());
			currentPath = currentDir->path.c_str();
		}
		else {
			std::list<RE_FileSystem::RE_Directory*> folders = currentDir->FromParentToThis();
			for (auto dir : folders) {
				if (dir == currentDir)
					ImGui::Text(currentDir->name.c_str());
				else {
					if (ImGui::Button(dir->name.c_str())) {
						toChange = dir;
					}
				}
				if(dir != *folders.rbegin()) ImGui::SameLine();
			}
		}
		ImGui::Separator();
		for (auto path : currentDir->tree) {
			switch (path->pType)
			{
			case RE_FileSystem::PathType::D_FILE:
				switch (path->AsFile()->fType)
				{
				case RE_FileSystem::FileType::F_META:
				{
					if (path->AsMeta()->resource) {
						ResourceContainer* res = App->resources->At(path->AsMeta()->resource);
						if (res->GetType() == R_SHADER && ImGui::Button(res->GetName()))
							App->resources->PushSelected(path->AsMeta()->resource, true);
					}
					break;
				}
				case RE_FileSystem::FileType::F_NONE:
					break;
				case RE_FileSystem::FileType::F_NOTSUPPORTED:
					if (selectingUndefFile) {
						if (ImGui::Button(std::string("Undefined: " + path->AsFile()->filename).c_str())) {
							*selectingUndefFile = path->path;
							selectingUndefFile = nullptr;
						}
					}
					else
						ImGui::Text(path->AsFile()->filename.c_str());

					break;
				default:
					if (ImGui::Button(path->AsFile()->filename.c_str()))
						App->resources->PushSelected(path->AsFile()->metaResource->resource, true);
					break;
				}
				break;
			case RE_FileSystem::PathType::D_FOLDER:
				if (ImGui::Button(path->AsDirectory()->name.c_str())) {
					toChange = path->AsDirectory();
				}
			}
		}

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (toChange) {
			currentDir = toChange;
			currentPath = currentDir->path.c_str();
		}
	}

	ImGui::End();
}

MaterialEditorWindow::MaterialEditorWindow(const char* name, bool start_active) : EditorWindow(name, start_active) 
{
	editingMaerial = new RE_Material();
	matName = "New Material";
}

MaterialEditorWindow::~MaterialEditorWindow()
{
	DEL(editingMaerial);
}

void MaterialEditorWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Material name:");
		ImGui::SameLine();
		ImGui::InputText("##matname", &matName);

		assetPath = "Assets/Materials/";
		assetPath += matName;
		assetPath += ".pupil";
		ImGui::Text("Save path: %s", assetPath.c_str());

		bool exits = App->fs->Exists(assetPath.c_str());
		if (exits) ImGui::Text("This material exits, change the name.");
	
		if (exits && !secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			if (!matName.empty() && !exits) {
				editingMaerial->SetName(matName.c_str());
				editingMaerial->SetAssetPath(assetPath.c_str());
				editingMaerial->SetType(Resource_Type::R_MATERIAL);
				editingMaerial->Save();
				App->resources->Reference((ResourceContainer*)editingMaerial);
				
				editingMaerial = new RE_Material();
				matName = "New Material";
			}
		}

		if (exits && !secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::Separator();
		editingMaerial->DrawMaterialEdit();

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
}

ShaderEditorWindow::ShaderEditorWindow(const char* name, bool start_active) : EditorWindow(name, start_active)
{
	editingShader = new RE_Shader();
	shaderName = "New Shader";
	vertexPath = "";
	fragmentPath = "";
	geometryPath = "";
}

ShaderEditorWindow::~ShaderEditorWindow()
{
	DEL(editingShader);
}

void ShaderEditorWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Shader name:");
		ImGui::SameLine();
		ImGui::InputText("##shadername", &shaderName);

		assetPath = "Assets/Shaders/";
		assetPath += shaderName;
		assetPath += ".meta";
		ImGui::Text("Save path: %s", assetPath.c_str());

		static bool compilePass = false;

		bool exits = App->fs->Exists(assetPath.c_str());
		if (exits) ImGui::Text("This shader exits, change the name.");
		bool neededVertexAndFragment = (vertexPath.empty() || fragmentPath.empty());
		if (neededVertexAndFragment) ImGui::Text("The vertex or fragment file path is empty.");

		static std::string* waitingPath = nullptr;
		if (waitingPath) ImGui::Text("Select Shader file from assets panel");

		bool pop2 = false;
		if (!compilePass) {
			ImGui::Text("Nedded pass the compile test.");
			pop2 = true;
		}
		if ((neededVertexAndFragment || exits || pop2) && !secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			if (!shaderName.empty() && !exits) {
				editingShader->SetName(shaderName.c_str());
				editingShader->SetType(Resource_Type::R_SHADER);
				editingShader->SetPaths(vertexPath.c_str(), fragmentPath.c_str(), (!geometryPath.empty()) ? geometryPath.c_str() : nullptr);
				editingShader->SaveMeta();
				App->resources->Reference((ResourceContainer*)editingShader);

				editingShader = new RE_Shader();
				shaderName = "New Shader";
				vertexPath = "";
				fragmentPath = "";
				geometryPath = "";
				compilePass = false;
			}
		}

		if ((neededVertexAndFragment || exits || pop2) && !secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::Separator();
		ImGui::Text("The files that contains the script is an undefined file.");
		ImGui::Text("Select the script type and undefined file on panel assets.");
		ImGui::Text("When activate the selection, the undefied files from assets panel can be selected.");

		bool pop = (waitingPath);
		if (pop && !secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::Separator();
		ImGui::Text("Vertex Shader Path:\n%s", (vertexPath.empty()) ? "No path." : vertexPath.c_str());
		if (ImGui::Button("Select vertex path")) {
			vertexPath.clear();
			waitingPath = &vertexPath;
			App->editor->SelectUndefinedFile(waitingPath);
			compilePass = false;
		}
		ImGui::SameLine();
		if(ImGui::Button("Clear vertex")) vertexPath.clear();

		ImGui::Separator();
		ImGui::Text("Fragment Shader Path:\n%s", (fragmentPath.empty()) ? "No path." : fragmentPath.c_str());
		if (ImGui::Button("Select fragment path")) {
			fragmentPath.clear();
			waitingPath = &fragmentPath;
			App->editor->SelectUndefinedFile(waitingPath);
			compilePass = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear fragment")) fragmentPath.clear();

		ImGui::Separator();
		ImGui::Text("Geometry Shader Path:\n%s", (geometryPath.empty()) ? "No path." : geometryPath.c_str());
		if (ImGui::Button("Select geometry path")) {
			geometryPath.clear();
			waitingPath = &geometryPath;
			App->editor->SelectUndefinedFile(waitingPath);
			compilePass = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear geometry")) geometryPath.clear();

		ImGui::Separator();
		if (neededVertexAndFragment) ImGui::Text("The vertex or fragment file path is empty.");

		if (neededVertexAndFragment && !pop && !secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (!compilePass && ImGui::Button("Compile Test")) {
			App->handlerrors->StartHandling();

			uint sID = 0;
			if (App->shaders->LoadFromAssets(&sID, vertexPath.c_str(), fragmentPath.c_str(), (!geometryPath.empty()) ? geometryPath.c_str() : nullptr, true))
				compilePass = true;
			//Error at log  that buffer is too small for show the error
			//else
			//	LOG_ERROR("Shader Compilation Error:\n%s", App->shaders->GetShaderError());
				
			App->handlerrors->StopHandling();
			if (App->handlerrors->AnyErrorHandled())
				App->handlerrors->ActivatePopUp();
		}

		if (secondary || pop || neededVertexAndFragment) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (waitingPath && !waitingPath->empty()) waitingPath = nullptr;
	}

	ImGui::End();
}
