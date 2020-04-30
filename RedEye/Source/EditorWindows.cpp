#include "EditorWindows.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "ModuleInput.h"
#include "ModuleRenderer3d.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "RE_ThumbnailManager.h"
#include "RE_ResourceManager.h"

#include "RE_CameraManager.h"
#include "TimeManager.h"
#include "OutputLog.h"
#include "RE_HandleErrors.h"

#include "ImGui/misc/cpp/imgui_stdlib.h"
#include "ImGui/imgui_internal.h"
#include "ImGuiColorTextEdit/TextEditor.h"
#include "SDL2/include/SDL_scancode.h"

std::queue<Event> events_queue;

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

				eastl::map<eastl::string, unsigned int>::iterator it = App->log->callers.begin();
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

		eastl::list<RE_Log>::iterator it = App->log->logHistory.begin();

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

	eastl::list<RE_Log>::iterator it = App->log->logHistory.begin();

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
			eastl::list<SoftwareInfo>::iterator it = sw_info.begin();
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
						eastl::string button_name = "Open ";
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

///////   Play Pause   ////////////////////////////////////////////
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

///////   PopUp Window   ////////////////////////////////////////////
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

///////   Assets Window   ////////////////////////////////////////////
AssetsWindow::AssetsWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}

AssetsWindow::~AssetsWindow()
{
}

const char* AssetsWindow::GetCurrentDirPath() const
{
	return currentPath;
}

void AssetsWindow::SelectUndefined(eastl::string* toFill)
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
			eastl::list<RE_FileSystem::RE_Directory*> folders = currentDir->FromParentToThis();
			for (auto dir : folders) {
				if (dir == currentDir)
					ImGui::Text(currentDir->name.c_str());
				else {
					if (ImGui::Button(dir->name.c_str())) {
						toChange = dir;
					}
				}
				if (dir != *folders.rbegin()) {
					ImGui::SameLine();
				}
			}
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100);
		static float iconsSize = 100;
		ImGui::SliderFloat("Icons size", &iconsSize, 25, 256, "%.0f");
		if (selectingUndefFile) {
			ImGui::SameLine();
			if (ImGui::Button("Cancel selection")) {
				selectingUndefFile = nullptr;
			}
		}
		ImGui::Separator();

		float width = ImGui::GetWindowWidth();
		int itemsColum = width / iconsSize;
		eastl::stack<RE_FileSystem::RE_Path*> filesToDisplay = currentDir->GetDisplayingFiles();

		ImGui::Columns(itemsColum, NULL, false);
		eastl::string idName = "#AssetImage";
		uint idCount = 0;
		while (!filesToDisplay.empty()) {
			RE_FileSystem::RE_Path* p = filesToDisplay.top();
			filesToDisplay.pop();
			eastl::string id = idName + eastl::to_string(idCount++);
			ImGui::PushID(id.c_str());
			switch (p->pType)
			{
			case RE_FileSystem::PathType::D_FOLDER:
				if (ImGui::ImageButton((void*)App->thumbnail->GetFolderID(), { iconsSize, iconsSize }, { 0.0, 1.0 }, { 1.0, 0.0 }, 0))
					toChange = p->AsDirectory();
				ImGui::PopID();
				ImGui::Text(p->AsDirectory()->name.c_str());
				break;
			case RE_FileSystem::PathType::D_FILE:
				switch (p->AsFile()->fType)
				{
				case RE_FileSystem::FileType::F_META:
				{
					ResourceContainer* res = App->resources->At(p->AsMeta()->resource);
					if (ImGui::ImageButton((void*)App->thumbnail->GetShaderFileID(), { iconsSize, iconsSize }, { 0.0, 1.0 }, { 1.0, 0.0 }, 0))
						App->resources->PushSelected(res->GetMD5(), true);
					if (ImGui::BeginDragDropSource()) {
						ImGui::SetDragDropPayload("#ShadereReference", &p->AsMeta()->resource, sizeof(const char**));
						ImGui::Image((void*)App->thumbnail->GetShaderFileID(), { 50,50 }, { 0.0, 1.0 }, { 1.0, 0.0 });
						ImGui::EndDragDropSource();
					}
					ImGui::PopID();
					ImGui::Text(res->GetName());
				}
					break;
				case RE_FileSystem::FileType::F_NOTSUPPORTED:
				{
					bool pop = (!selectingUndefFile && !secondary);
					if (pop) {
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
					}

					if (ImGui::ImageButton((selectingUndefFile) ? (void*)App->thumbnail->GetSelectFileID() : (void*)App->thumbnail->GetFileID(), { iconsSize, iconsSize }, { 0.0, 1.0 }, { 1.0, 0.0 }, (selectingUndefFile) ? -1 : 0)) {
						if (selectingUndefFile) {
							*selectingUndefFile = p->path;
							selectingUndefFile = nullptr;
						}
					}
					ImGui::PopID();
					ImGui::Text(p->AsFile()->filename.c_str());

					if (pop) {
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();
					}
				}
					break;
				default:
				{
					ResourceContainer* res = App->resources->At(p->AsFile()->metaResource->resource);
					if (ImGui::ImageButton((void*)App->thumbnail->At(res->GetMD5()), { iconsSize, iconsSize }, { 0.0, 1.0 }, { 1.0, 0.0 }, 0))
						App->resources->PushSelected(res->GetMD5(), true);
					ImGui::PopID();

					eastl::string dragID("#");
					
					switch (res->GetType())
					{
					case R_TEXTURE:
						dragID += "Texture";
						break;

					case R_PREFAB:
						dragID += "Prefab";
						break;

					case R_SKYBOX:
						dragID += "Skybox";
						break;

					case R_MATERIAL:
						dragID += "Material";
						break;

					case R_MODEL:
						dragID += "Model";
						break;

					case R_SCENE:
						dragID += "Scene";
						break;

					}
					dragID += "Reference";

					if (ImGui::BeginDragDropSource()) {
						ImGui::SetDragDropPayload(dragID.c_str(), &p->AsFile()->metaResource->resource, sizeof(const char**));
						ImGui::Image((void*)App->thumbnail->At(p->AsFile()->metaResource->resource), { 50,50 }, { 0.0, 1.0 }, { 1.0, 0.0 });
						ImGui::EndDragDropSource();
					}

					ImGui::Text(p->AsFile()->filename.c_str());

				}
					break;
				}
				break;
			}
			ImGui::NextColumn();
		}
		ImGui::Columns(1);

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

SceneEditorWindow::SceneEditorWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}

SceneEditorWindow::~SceneEditorWindow()
{
}

void SceneEditorWindow::UpdateViewPort()
{
	RE_CameraManager::EditorCamera()->GetTargetViewPort(viewport);
	viewport.x = (width - viewport.z) * 0.5f;
	viewport.y = (heigth - viewport.w) * 0.5f + 20;
}

void SceneEditorWindow::Recalc()
{
	recalc = true;
}

void SceneEditorWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		static int lastWidht = 0;
		static int lastHeight = 0;
		ImVec2 size = ImGui::GetWindowSize();
		width = (int)size.x;
		heigth = (int)size.y - 28;
		if (recalc || lastWidht != width || lastHeight != heigth) {
			Event::Push(RE_EventType::EDITORWINDOWCHANGED, App->renderer3d, Cvar(lastWidht = width), Cvar(lastHeight = heigth));
			Event::Push(RE_EventType::EDITORWINDOWCHANGED, App->editor);
			recalc = false;
		}

		isWindowSelected = (ImGui::IsWindowHovered() && ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow));

		ImGui::SetCursorPos({ viewport.x, viewport.y });
		ImGui::Image((void*)App->renderer3d->GetRenderedEditorSceneTexture(), { viewport.z, viewport.w }, { 0.0, 1.0 }, { 1.0, 0.0 });
		
		if(isWindowSelected && App->input->GetKey(SDL_SCANCODE_LALT) == KEY_IDLE && App->input->GetMouse().GetButton(1) == KEY_STATE::KEY_DOWN){
			ImVec2 mousePosOnThis = ImGui::GetMousePos();
			mousePosOnThis.x = (mousePosOnThis.x - ImGui::GetCursorScreenPos().x + 4 < 0) ? 0 : mousePosOnThis.x - ImGui::GetCursorScreenPos().x + 4;
			mousePosOnThis.y = (ImGui::GetItemRectSize().y + mousePosOnThis.y - ImGui::GetCursorScreenPos().y + 4 < 0) ? 0 : ImGui::GetItemRectSize().y + mousePosOnThis.y - ImGui::GetCursorScreenPos().y + 4;
			Event::Push(EDITOR_SCENE_RAYCAST, App->editor, Cvar(mousePosOnThis.x), Cvar(mousePosOnThis.y));
		}

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
}

SceneGameWindow::SceneGameWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}

SceneGameWindow::~SceneGameWindow()
{
}

void SceneGameWindow::UpdateViewPort()
{
	RE_CameraManager::MainCamera()->GetTargetViewPort(viewport);
	viewport.x = (width - viewport.z) * 0.5f;
	viewport.y = (heigth - viewport.w) * 0.5f + 20;
}

void SceneGameWindow::Recalc()
{
	recalc = true;
}

void SceneGameWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		static int lastWidht = 0;
		static int lastHeight = 0;
		ImVec2 size = ImGui::GetWindowSize();
		width = (int)size.x;
		heigth = (int)size.y - 28;
		if (recalc || lastWidht != width || lastHeight != heigth) {
			Event::Push(RE_EventType::GAMEWINDOWCHANGED, App->renderer3d, Cvar(lastWidht = width), Cvar(lastHeight = heigth));
			Event::Push(RE_EventType::GAMEWINDOWCHANGED, App->editor);
			recalc = false;
		}

		isWindowSelected = (ImGui::IsWindowHovered() && ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow));

		ImGui::SetCursorPos({ viewport.x, viewport.y });
		ImGui::Image((void*)App->renderer3d->GetRenderedGameSceneTexture(), { viewport.z, viewport.w }, { 0.0, 1.0 }, { 1.0, 0.0 });

		if (isWindowSelected && App->input->GetMouse().GetButton(1) == KEY_STATE::KEY_DOWN) {
			ImVec2 mousePosOnThis = ImGui::GetMousePos();
			mousePosOnThis.x = mousePosOnThis.x - ImGui::GetCursorScreenPos().x;
			mousePosOnThis.y = (ImGui::GetItemRectSize().y + mousePosOnThis.y - ImGui::GetCursorScreenPos().y < 0) ? 0 : ImGui::GetItemRectSize().y + mousePosOnThis.y - ImGui::GetCursorScreenPos().y;

			//TODO RAYCAST

		}

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
}