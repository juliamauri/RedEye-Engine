#include "Event.h"

#include "ModuleEditor.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "RE_ConsoleLog.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "ModuleRenderer3D.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_CommandManager.h"
#include "RE_ThumbnailManager.h"
#include "RE_CameraManager.h"
#include "RE_ParticleEmitter.h"
#include "RE_ResourceManager.h"
#include "RE_ParticleManager.h"

#include "AboutWindow.h"
#include "AssetsWindow.h"
#include "ConfigWindow.h"
#include "ConsoleWindow.h"
#include "HierarchyWindow.h"
#include "PlayPauseWindow.h"
#include "PropertiesWindow.h"
#include "PopUpWindow.h"
#include "WwiseWindow.h"

#include "SceneEditorWindow.h"
#include "GameWindow.h"

#include "MaterialEditorWindow.h"
#include "SkyBoxEditorWindow.h"
#include "ShaderEditorWindow.h"
#include "TextEditorManagerWindow.h"
#include "WaterPlaneWindow.h"
#include "ParticleEmitterEditorWindow.h"

#include <ImGuiImpl/imgui_impl_sdl2.h>
#include <ImGuiImpl/imgui_stdlib.h>
#include <ImGuiImpl/imgui_impl_opengl3.h>
#include <ImGui/imgui_internal.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <EASTL/stack.h>
#include <EASTL/queue.h>

ModuleEditor::ModuleEditor() :
	popupWindow(new PopUpWindow),
	about(new AboutWindow)
{
	flags = static_cast<int>(Flag::SHOW_EDITOR);
}

ModuleEditor::~ModuleEditor()
{
	DEL(popupWindow)
	DEL(about)
}

#pragma region Module

bool ModuleEditor::Init()
{
	RE_PROFILE(RE_ProfiledFunc::Init, RE_ProfiledClass::ModuleEditor)
	RE_LOG("Initializing Module Editor");

	if (!InitializeImGui())
	{
		RE_LOG_ERROR("Error Initializing ImGui!");
		return false;
	}

	// Base Windows
	windows.push_back(console = new ConsoleWindow());
	windows.push_back(config = new ConfigWindow());
	windows.push_back(hierarchy = new HierarchyWindow());
	windows.push_back(properties = new PropertiesWindow());
	windows.push_back(play_pause = new PlayPauseWindow());

	// Resource Editors
	materialeditor = new MaterialEditorWindow();
	skyboxeditor = new SkyBoxEditorWindow();
	shadereditor = new ShaderEditorWindow();
	texteditormanager = new TextEditorManagerWindow();
	waterplaneWindow = new WaterPlaneWindow();

	return true;
}

bool ModuleEditor::Start()
{
	RE_PROFILE(RE_ProfiledFunc::Start, RE_ProfiledClass::ModuleEditor)
	windows.push_back(assets = new AssetsWindow());
	windows.push_back(wwise = new WwiseWindow());

	// Scene views
	rendered_windows.push_back(sceneEditorWindow = new SceneEditorWindow());
	rendered_windows.push_back(sceneGameWindow = new GameWindow());
	rendered_windows.push_back(particleEmitterWindow = new ParticleEmitterEditorWindow());

	RE_ThumbnailManager::Init();

	return true;
}

void ModuleEditor::PreUpdate()
{
	RE_PROFILE(RE_ProfiledFunc::PreUpdate, RE_ProfiledClass::ModuleEditor)
	ImGuizmo::SetOrthographic(sceneEditorWindow->GetCamera().IsOrthographic());
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void ModuleEditor::Update()
{
	RE_PROFILE(RE_ProfiledFunc::Update, RE_ProfiledClass::ModuleEditor)

	if (HasFlag(Flag::SHOW_EDITOR))
	{
		DrawMainMenuBar();
		DrawWindows();
	}

	CheckEditorInputs();
	UpdateEditorCameras();

	ImGui::End();
}

void ModuleEditor::CleanUp()
{
	RE_PROFILE(RE_ProfiledFunc::CleanUp, RE_ProfiledClass::ModuleEditor)
	
	RE_CommandManager::Clear();
	RE_ThumbnailManager::Clear();

	windows.clear();
	rendered_windows.clear();

	DEL(materialeditor)
	DEL(skyboxeditor)
	DEL(shadereditor)
	DEL(texteditormanager)
	DEL(waterplaneWindow)

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void ModuleEditor::DrawEditor()
{
	for (auto& window : rendered_windows)
		if (ImGui::TreeNodeEx(window->Name(), ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
		{
			window->DrawEditor();
			ImGui::TreePop();
		}
}

void ModuleEditor::Load()
{
	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::ModuleWindow)

	// ImGui Settings
	RE_FileBuffer _load("imgui.ini");
	if (_load.Load()) ImGui::LoadIniSettingsFromMemory(_load.GetBuffer(), _load.GetSize());

	// Editor Flags
	RE_LOG_SECONDARY("Loading Editor propieties from config:");
	RE_Json* node = RE_FS->ConfigNode("Editor");
	node->PullBool("Show_Editor", HasFlag(Flag::SHOW_EDITOR)) ? AddFlag(Flag::SHOW_EDITOR) : RemoveFlag(Flag::SHOW_EDITOR);
	node->PullBool("Show_Imgui_Demo", HasFlag(Flag::SHOW_IMGUI_DEMO)) ? AddFlag(Flag::SHOW_IMGUI_DEMO) : RemoveFlag(Flag::SHOW_IMGUI_DEMO);

	// Rendered Windows
	for (auto& window : rendered_windows)
		window->Load(node->PullJObject(window->Name()));

	DEL(node)
}

void ModuleEditor::Save() const
{
	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::ModuleWindow)

	// ImGui Settings
	size_t _size = 0;
	const char* buff = ImGui::SaveIniSettingsToMemory(&_size);
	RE_FileBuffer _save("imgui.ini");
	_save.Save(const_cast<char*>(buff), _size);

	// Editor Flags
	RE_Json* node = RE_FS->ConfigNode("Editor");
	node->Push("Show_Editor", HasFlag(Flag::SHOW_EDITOR));
	node->Push("Show_Imgui_Demo", HasFlag(Flag::SHOW_IMGUI_DEMO));

	// Rendered Windows
	for (auto& window : rendered_windows)
		window->Save(node->PushJObject(window->Name()));

	DEL(node)
}

#pragma endregion

#pragma region Events

void ModuleEditor::RecieveEvent(const Event& e)
{
	switch (e.type)
	{
	case RE_EventType::UPDATE_SCENE_WINDOWS: e.data1.AsGO() ? sceneGameWindow->Recalc() : sceneEditorWindow->Recalc(); break;
	case RE_EventType::SCOPE_PROCEDURE_END: if (e.data1.AsBool()) popupWindow->PopUpError(); break;
	default:
		if (e.type > RE_EventType::CONSOLE_LOG_MIN && e.type < RE_EventType::CONSOLE_LOG_MAX)
		{
			uint category = static_cast<uint>(e.type) - static_cast<uint>(RE_EventType::CONSOLE_LOG_SEPARATOR);
			const char* text = e.data1.AsCharP();

			if (e.type >= RE_EventType::CONSOLE_LOG_SAVE_ERROR)
			{
				category -= 3;
				popupWindow->AppendScopedLog(text, e.type);
			}

			console->AppendLog(category, text, e.data2.AsCharP());
		}
		else RE_LOG("Unused Event at Module Editor");
		break;
	}
}

void ModuleEditor::HandleSDLEvent(SDL_Event* e) { ImGui_ImplSDL2_ProcessEvent(e); }

#pragma endregion

#pragma region Draws

void ModuleEditor::DrawEditorWindows() const
{
	RE_PROFILE(RE_ProfiledFunc::DrawEditor, RE_ProfiledClass::ModuleEditor)

	for (auto& window : rendered_windows)
		window->DrawWindow(true);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
		return;

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	SDL_GL_MakeCurrent(RE_WINDOW->GetWindow(), RE_RENDER->GetWindowContext());
}

void ModuleEditor::RenderWindowFBOs() const
{
	for (auto& window : rendered_windows)
		window->RenderFBO();
}

void ModuleEditor::DrawHeriarchy()
{
	if (!RE_SCENE) return;

	GO_UID to_select = 0;
	GO_UID goToDelete_uid = 0;

	const RE_GameObject* root = RE_SCENE->GetRootCPtr();
	GO_UID root_uid = root->GetUID();

	if (root->ChildCount() > 0)
	{
		eastl::stack<RE_GameObject*> gos;
		for (auto child : root->GetChildsPtrReversed()) gos.push(child);

		uint count = 0;
		while (!gos.empty())
		{
			RE_GameObject* go = gos.top();
			gos.pop();

			GO_UID go_uid = go->GetUID();
			bool is_leaf = (go->ChildCount() == 0);

			ImGui::PushID(eastl::string("#HierarchyGOID" + eastl::to_string(count++)).c_str());
			if (ImGui::TreeNodeEx(go->name.c_str(), ImGuiTreeNodeFlags_(selected == go_uid ?
				(is_leaf ? ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_Leaf :
					ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick) :
				(is_leaf ? ImGuiTreeNodeFlags_Leaf :
					ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick))))
			{
				if (is_leaf) ImGui::TreePop();
				else for (auto child : go->GetChildsPtrReversed()) gos.push(child);
			}
			ImGui::PopID();

			if (ImGui::IsItemClicked()) to_select = go_uid;

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Create Prefab")) popupWindow->PopUpPrefab(go);
				ImGui::Separator();
				if (ImGui::MenuItem("Destroy GameObject")) goToDelete_uid = go_uid;
				ImGui::Separator();
				DrawGameObjectItems(go_uid);
				ImGui::EndPopup();
			}

			if (go->IsLastChild() && go->GetParentUID() != root_uid) ImGui::TreePop();
		}
	}

	if (to_select) SetSelected(to_select);
	if (goToDelete_uid)
	{
		RE_INPUT->Push(RE_EventType::DESTROY_GO, RE_SCENE, goToDelete_uid);
		if (selected == goToDelete_uid || RE_SCENE->GetGOPtr(goToDelete_uid)->isParent(selected)) selected = 0;
	}
}

#pragma endregion

#pragma region GO Selection

void ModuleEditor::SetSelected(GO_UID go, bool force_focus)
{
	selected = go;
	RE_RES->PopSelected(true);

	if (force_focus ||
		(sceneEditorWindow->HasFlag(SceneEditorWindow::Flag::FOCUS_GO_ON_SELECT) && selected))
		sceneEditorWindow->Focus();
}

void ModuleEditor::DuplicateSelectedObject()
{
	if (!selected) return;

	const RE_GameObject* sel_go = RE_SCENE->GetGOCPtr(selected);
	RE_SCENE->GetGOPtr(RE_SCENE->GetScenePool()->CopyGOandChilds(sel_go, sel_go->GetParentUID(), true))->UseResources();
}

#pragma endregion

#pragma region Flags

void ModuleEditor::CheckboxFlag(const char* label, Flag flag)
{
	bool tmp = HasFlag(flag);
	if (ImGui::Checkbox(label, &tmp))
		tmp ? AddFlag(flag) : RemoveFlag(flag);
}

#pragma endregion

#pragma region Editor Windows

const char* ModuleEditor::GetAssetsPanelPath() const { return assets->GetCurrentDirPath(); }
void ModuleEditor::SelectUndefinedFile(eastl::string* toSelect) const { assets->SelectUndefined(toSelect); }

void ModuleEditor::OpenTextEditor(const char* filePath, eastl::string* filePathStr, const char* shadertTemplate, bool* open)
{
	texteditormanager->PushEditor(filePath, filePathStr, shadertTemplate, open);
}

void ModuleEditor::ReportSoftawe(const char* name, const char* version, const char* website) const
{
	about->sw_info.push_back({ name, version ? version : "", website ? website : "" });
}

void ModuleEditor::SetPopUpFocus(bool focus)
{
	if (focus && !HasFlag(Flag::POPUP_IS_FOCUSED)) AddFlag(Flag::POPUP_IS_FOCUSED);
	else if (!focus && HasFlag(Flag::POPUP_IS_FOCUSED)) RemoveFlag(Flag::POPUP_IS_FOCUSED);
}

#pragma endregion

#pragma region Init

bool ModuleEditor::InitializeImGui()
{
	RE_LOG_SECONDARY("Init ImGui");
	if (!IMGUI_CHECKVERSION())
	{
		RE_LOG_ERROR("ABI incompatibility error between caller code and compiled version of Dear ImGui!");
		return false;
	}

	ImGui::CreateContext();
	ApplyRedeyeStyling();

	if (!ImGui_ImplSDL2_InitForOpenGL(RE_WINDOW->GetWindow(), RE_RENDER->GetWindowContext()))
	{
		RE_LOG_ERROR("ImGui could not SDL2_InitForOpenGL!");
		return false;
	}

	if (!ImGui_ImplOpenGL3_Init())
	{
		RE_LOG_ERROR("ImGui could not OpenGL3_Init!");
		return false;
	}

	RE_SOFT_NVS("ImGui", IMGUI_VERSION, "https://github.com/ocornut/imgui");

	return true;
}

void ModuleEditor::ApplyRedeyeStyling()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.37f, 0.14f, 0.14f, 0.67f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.39f, 0.20f, 0.20f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.56f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.17f, 0.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.33f, 0.35f, 0.36f, 0.53f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.76f, 0.28f, 0.44f, 0.67f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.47f, 0.47f, 0.67f);
	colors[ImGuiCol_Separator] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.07f, 0.07f, 0.51f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.86f, 0.23f, 0.43f, 0.67f);
	colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.19f, 0.19f, 0.57f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.05f, 0.05f, 0.90f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.13f, 0.13f, 0.74f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.47f, 0.47f, 0.47f, 0.47f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

#pragma endregion

#pragma region Editor Update

void ModuleEditor::DrawWindows()
{
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None; // | ImGuiDockNodeFlags_PassthruCentralNode;
	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background 
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", (bool*)true, window_flags);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	popupWindow->DrawWindow(true);

	// Draw Windows
	bool popUpFocus = HasFlag(Flag::POPUP_IS_FOCUSED);
	for (auto window : windows) window->DrawWindow(true, popUpFocus);
	if (about) about->DrawWindow(true, popUpFocus); // (not in windows' list)

	// Draw Editors
	materialeditor->DrawWindow(true, popUpFocus);
	shadereditor->DrawWindow(true, popUpFocus);
	skyboxeditor->DrawWindow(true, popUpFocus);
	waterplaneWindow->DrawWindow(true, popUpFocus);
	particleEmitterWindow->DrawWindow(true, popUpFocus);
	texteditormanager->DrawWindow(false, popUpFocus);
}

void ModuleEditor::CheckEditorInputs()
{
	// Toggle show editor on F1
	if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_F1)->Down)
		HasFlag(Flag::SHOW_EDITOR) ? RemoveFlag(Flag::SHOW_EDITOR) : AddFlag(Flag::SHOW_EDITOR);

	// Undo/Redo Commands
	if (!ImGui::GetKeyData(ImGuiKey::ImGuiKey_LeftCtrl)->Down) return;
	if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_Z)->Down) RE_CommandManager::Undo();
	if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_Y)->Down) RE_CommandManager::Redo();
}

void ModuleEditor::UpdateEditorCameras()
{
	RE_PROFILE(RE_ProfiledFunc::EditorCamera, RE_ProfiledClass::ModuleEditor)
	sceneEditorWindow->UpdateCamera();
	particleEmitterWindow->UpdateCamera();
}

#pragma endregion

#pragma region Main Menu Bar

void ModuleEditor::DrawMainMenuBar()
{
	if (!ImGui::BeginMainMenuBar()) return;

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem(" New Scene"))
		{
			if (RE_SCENE->HasChanges()) RE_EDITOR->popupWindow->PopUpSaveScene(false, true);
			else RE_SCENE->NewEmptyScene();
		}
		if (ImGui::MenuItem(" Save Scene") && RE_SCENE->HasChanges())
		{
			if (RE_SCENE->isNewScene()) RE_EDITOR->popupWindow->PopUpSaveScene();
			else RE_SCENE->SaveScene();
		}
		if (ImGui::MenuItem(" Exit", "	Esc"))
		{
			if (RE_SCENE->HasChanges()) RE_EDITOR->popupWindow->PopUpSaveScene(true);
			else RE_INPUT->Push(RE_EventType::REQUEST_QUIT, App);
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Undo", "LCTRL + Z", false, RE_CommandManager::CanUndo()))
			RE_CommandManager::Undo();

		if (ImGui::MenuItem("Redo", "LCTRL + Y", false, RE_CommandManager::CanRedo()))
			RE_CommandManager::Redo();

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Assets"))
	{
		if (ImGui::BeginMenu("Create")) {

			if (ImGui::MenuItem("Material", materialeditor->IsActive() ? "Hide" : "Open"))
				materialeditor->ToggleActive();

			if (ImGui::MenuItem("Shader", shadereditor->IsActive() ? "Hide" : "Open"))
				shadereditor->ToggleActive();

			if (ImGui::MenuItem("Skybox", skyboxeditor->IsActive() ? "Hide" : "Open"))
				skyboxeditor->ToggleActive();

			if (ImGui::MenuItem("Water Resources", waterplaneWindow->IsActive() ? "Hide" : "Open"))
				waterplaneWindow->ToggleActive();

			if (ImGui::MenuItem("Particle Resources"))
				particleEmitterWindow->StartEditing(RE_ParticleManager::Allocate(*(new RE_ParticleEmitter())), nullptr);

			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Gameobject"))
	{
		DrawGameObjectItems();
		ImGui::EndMenu();
	}

	// View
	if (ImGui::BeginMenu("View"))
	{
		for (auto window : windows)
			if (ImGui::MenuItem(window->Name(), window->IsActive() ? "Hide" : "Open"))
				window->ToggleActive();

		ImGui::EndMenu();
	}

	// Help
	if (ImGui::BeginMenu("Help"))
	{
		bool tmp = HasFlag(Flag::SHOW_IMGUI_DEMO);
		if (ImGui::MenuItem(tmp ? "Close Gui Demo" : "Open Gui Demo"))
			tmp ? RemoveFlag(Flag::SHOW_IMGUI_DEMO) : AddFlag(Flag::SHOW_IMGUI_DEMO);

		if (ImGui::MenuItem("Documentation"))
			BROWSER("https://github.com/juliamauri/RedEye-Engine/wiki");
		if (ImGui::MenuItem("Download Latest"))
			BROWSER("https://github.com/juliamauri/RedEye-Engine/releases");
		if (ImGui::MenuItem("Report a Bug"))
			BROWSER("https://github.com/juliamauri/RedEye-Engine/issues");
		if (ImGui::MenuItem("About", about->IsActive() ? "Hide" : "Open"))
			about->ToggleActive();

		ImGui::EndMenu();
	}

	if (HasFlag(Flag::SHOW_IMGUI_DEMO)) ImGui::ShowDemoWindow();

	ImGui::EndMainMenuBar();
}

void ModuleEditor::DrawGameObjectItems(GO_UID parent)
{
	if (ImGui::BeginMenu("Primitive"))
	{
		if (ImGui::MenuItem("Grid"))		 RE_SCENE->CreatePrimitive(RE_Component::Type::GRID, parent);
		if (ImGui::MenuItem("Cube"))		 RE_SCENE->CreatePrimitive(RE_Component::Type::CUBE, parent);
		if (ImGui::MenuItem("Dodecahedron")) RE_SCENE->CreatePrimitive(RE_Component::Type::DODECAHEDRON, parent);
		if (ImGui::MenuItem("Tetrahedron"))	 RE_SCENE->CreatePrimitive(RE_Component::Type::TETRAHEDRON, parent);
		if (ImGui::MenuItem("Octohedron"))	 RE_SCENE->CreatePrimitive(RE_Component::Type::OCTOHEDRON, parent);
		if (ImGui::MenuItem("Icosahedron"))  RE_SCENE->CreatePrimitive(RE_Component::Type::ICOSAHEDRON, parent);
		if (ImGui::MenuItem("Point"))		 RE_SCENE->CreatePrimitive(RE_Component::Type::POINT, parent);
		if (ImGui::MenuItem("Plane"))		 RE_SCENE->CreatePrimitive(RE_Component::Type::PLANE, parent);
		// if (ImGui::MenuItem("Frustum"))	 RE_SCENE->CreatePrimitive(RE_Component::Type::C_FUSTRUM, parent);
		if (ImGui::MenuItem("Sphere"))		 RE_SCENE->CreatePrimitive(RE_Component::Type::SPHERE, parent);
		if (ImGui::MenuItem("Cylinder"))	 RE_SCENE->CreatePrimitive(RE_Component::Type::CYLINDER, parent);
		if (ImGui::MenuItem("HemiSphere"))	 RE_SCENE->CreatePrimitive(RE_Component::Type::HEMISHPERE, parent);
		if (ImGui::MenuItem("Torus"))		 RE_SCENE->CreatePrimitive(RE_Component::Type::TORUS, parent);
		if (ImGui::MenuItem("Trefoil Knot")) RE_SCENE->CreatePrimitive(RE_Component::Type::TREFOILKNOT, parent);
		if (ImGui::MenuItem("Rock"))		 RE_SCENE->CreatePrimitive(RE_Component::Type::ROCK, parent);

		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("Camera"))			RE_SCENE->CreateCamera(parent);
	if (ImGui::MenuItem("Light"))			RE_SCENE->CreateLight(parent);
	if (ImGui::MenuItem("Max Lights"))		RE_SCENE->CreateMaxLights(parent);
	if (ImGui::MenuItem("Water"))			RE_SCENE->CreateWater(parent);
	if (ImGui::MenuItem("Particle System"))	RE_SCENE->CreateParticleSystem(parent);
}

#pragma endregion