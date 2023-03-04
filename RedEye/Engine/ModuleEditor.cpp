#include "Event.h"

#include "ModuleEditor.h"

#include "RE_Profiler.h"
#include "RE_ConsoleLog.h"
#include "RE_Time.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "ModuleRenderer3D.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_ResourceManager.h"
#include "RE_CommandManager.h"
#include "RE_ThumbnailManager.h"
#include "RE_CameraManager.h"
#include "RE_ParticleEmitter.h"
#include "RE_Memory.h"

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
	commands(new RE_CommandManager),
	thumbnails(new RE_ThumbnailManager),
	popupWindow(new PopUpWindow),
	about(new AboutWindow)
{
	grid_size[0] = grid_size[1] = 1.f;

	all_aabb_color[0] = 0.f;
	all_aabb_color[1] = 1.f;
	all_aabb_color[2] = 0.f;

	sel_aabb_color[0] = 1.f;
	sel_aabb_color[1] = .5f;
	sel_aabb_color[2] = 0.f;

	quad_tree_color[0] = 1.f;
	quad_tree_color[1] = 1.f;
	quad_tree_color[2] = 0.f;

	frustum_color[0] = 0.f;
	frustum_color[1] = 1.f;
	frustum_color[2] = 1.f;
}

ModuleEditor::~ModuleEditor()
{
	DEL(popupWindow);
	DEL(about);

	DEL(commands);
	DEL(thumbnails);
}

bool ModuleEditor::Init()
{
	bool ret = false;
	RE_PROFILE(PROF_Init, PROF_ModuleEditor);
	RE_LOG("Initializing Module Editor");

	// ImGui
	RE_LOG_SECONDARY("Init ImGui");
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
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


	if (ImGui_ImplSDL2_InitForOpenGL(RE_WINDOW->GetWindow(), RE_RENDER->GetWindowContext()))
	{
		if (ImGui_ImplOpenGL3_Init())
		{
			RE_SOFT_NVS("ImGui", IMGUI_VERSION, "https://github.com/ocornut/imgui");
			
			materialeditor = new MaterialEditorWindow();
			skyboxeditor = new SkyBoxEditorWindow();
			shadereditor = new ShaderEditorWindow();
			texteditormanager = new TextEditorManagerWindow();
			waterplaneWindow = new WaterPlaneWindow();

			windows.push_back(console = new ConsoleWindow());
			windows.push_back(config = new ConfigWindow());
			windows.push_back(hierarchy = new HierarchyWindow());
			windows.push_back(properties = new PropertiesWindow());
			windows.push_back(play_pause = new PlayPauseWindow());

			sceneEditorWindow = new SceneEditorWindow();
			sceneGameWindow = new GameWindow();
			particleEmitterWindow = new ParticleEmitterEditorWindow();
			
			ret = true;
		}
		else RE_LOG_ERROR("ImGui could not OpenGL3_Init!");
	}
	else RE_LOG_ERROR("ImGui could not SDL2_InitForOpenGL!");

	return ret;
}

bool ModuleEditor::Start()
{
	RE_PROFILE(PROF_Start, PROF_ModuleEditor);
	windows.push_back(assets = new AssetsWindow());
	windows.push_back(wwise = new WwiseWindow());

	grid = new RE_CompGrid();
	grid->SetParent(0ull);
	grid->GridSetUp(50);

	// FOCUS CAMERA
	GO_UID first = RE_SCENE->GetRootCPtr()->GetFirstChildUID();
	if (first) SetSelected(first);

	thumbnails->Init();

	return true;
}

void ModuleEditor::PreUpdate()
{
	RE_PROFILE(PROF_PreUpdate, PROF_ModuleEditor);
	ImGuizmo::SetOrthographic(!RE_SCENE->cams->EditorCamera()->isPrespective());
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void ModuleEditor::Update()
{
	RE_PROFILE(PROF_Update, PROF_ModuleEditor);
	if (show_all)
	{
		// Main Menu Bar
		if (ImGui::BeginMainMenuBar())
		{
			// File
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

			//Edit
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "LCTRL + Z", false, commands->CanUndo()))
					commands->Undo();

				if (ImGui::MenuItem("Redo", "LCTRL + Y", false, commands->CanRedo()))
					commands->Redo();

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
						particleEmitterWindow->StartEditing(new RE_ParticleEmitter(true), nullptr);

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
				if (ImGui::MenuItem(show_demo ? "Close Gui Demo" : "Open Gui Demo"))
					show_demo = !show_demo;
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

			if (show_demo) ImGui::ShowDemoWindow();

			ImGui::EndMainMenuBar();
		}

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

		if (popupWindow->IsActive()) popupWindow->DrawWindow();

		// Draw Windows
		for (auto window : windows)
			if (window->IsActive())
				window->DrawWindow(popUpFocus);

		if (about && about->IsActive())
			about->DrawWindow(popUpFocus); // (not in windows' list)

		if (materialeditor->IsActive()) materialeditor->DrawWindow(popUpFocus);
		if (shadereditor->IsActive()) shadereditor->DrawWindow(popUpFocus);
		if (skyboxeditor->IsActive()) skyboxeditor->DrawWindow(popUpFocus);
		if (waterplaneWindow->IsActive()) waterplaneWindow->DrawWindow(popUpFocus);
		if (particleEmitterWindow->IsActive()) particleEmitterWindow->DrawWindow(popUpFocus);

		texteditormanager->DrawWindow(popUpFocus);
	}
	
	

	// Toggle show editor on F1
	if(ImGui::GetKeyData(ImGuiKey::ImGuiKey_F1)->Down)
		show_all = !show_all;

	// CAMERA CONTROLS
	UpdateCamera();

	if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_LeftCtrl)->Down)
	{
		if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_Z)->Down)
			commands->Undo();

		if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_Y)->Down)
			commands->Redo();
	}

	ImGui::End();
}

void ModuleEditor::CleanUp()
{
	RE_PROFILE(PROF_CleanUp, PROF_ModuleEditor);
	commands->Clear();
	thumbnails->Clear();

	windows.clear();

	DEL(materialeditor);
	DEL(skyboxeditor);
	DEL(shadereditor);

	DEL(texteditormanager);
	DEL(waterplaneWindow);

	DEL(sceneEditorWindow);
	DEL(sceneGameWindow);

	DEL(grid);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void ModuleEditor::RecieveEvent(const Event& e)
{
	switch (e.type)
	{
	case RE_EventType::PARTRICLEEDITORWINDOWCHANGED: particleEmitterWindow->UpdateViewPort(); break;
	case RE_EventType::EDITORWINDOWCHANGED: sceneEditorWindow->UpdateViewPort(); break;
	case RE_EventType::GAMEWINDOWCHANGED: sceneGameWindow->UpdateViewPort(); break;
	case RE_EventType::UPDATE_SCENE_WINDOWS:
	{
		if(e.data1.AsGO()) sceneGameWindow->Recalc();
		else sceneEditorWindow->Recalc();
		break;
	}
	case RE_EventType::EDITOR_SCENE_RAYCAST:
	{
		// Mouse Pick
		RE_CompCamera* camera = RE_CameraManager::EditorCamera();
		float width, height;
		camera->GetTargetWidthHeight(width, height);

		RE_PROFILE(PROF_CameraRaycast, PROF_ModuleEditor);
		GO_UID hit = RE_SCENE->RayCastGeometry(
			math::Ray(camera->GetFrustum().UnProjectLineSegment(
			(e.data1.AsFloat() -(width / 2.0f)) / (width / 2.0f),
				((height - e.data2.AsFloat()) - (height / 2.0f)) / (height / 2.0f))));

		if (hit) SetSelected(hit);

		break;
	}
	case RE_EventType::SCOPE_PROCEDURE_END:
	{
		if (e.data1.AsBool()) popupWindow->PopUpError();
		break;
	}
	default:
	{
		if (e.type > RE_EventType::CONSOLE_LOG_MIN && e.type < RE_EventType::CONSOLE_LOG_MAX)
		{
			unsigned int category = static_cast<unsigned int>(e.type) - static_cast<unsigned int>(RE_EventType::CONSOLE_LOG_SEPARATOR);
			const char* text = e.data1.AsCharP();

			if (e.type >= RE_EventType::CONSOLE_LOG_SAVE_ERROR)
			{
				category -= 3u;
				popupWindow->AppendScopedLog(text, e.type);
			}

			console->AppendLog(category, text, e.data2.AsCharP());
		}
	}
	}
}

void ModuleEditor::Draw() const
{
	RE_PROFILE(PROF_DrawEditor, PROF_ModuleEditor);
	sceneEditorWindow->DrawWindow();
	sceneGameWindow->DrawWindow();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_Window* backup_current_window = RE_WINDOW->GetWindow();
		SDL_GLContext backup_current_context = RE_RENDER->GetWindowContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
	}
}

void ModuleEditor::DrawEditor()
{
	ImGui::DragFloat("Camera speed", &cam_speed, 0.1f, 0.1f, 100.0f, "%.1f");
	ImGui::DragFloat("Camera sensitivity", &cam_sensitivity, 0.01f, 0.01f, 1.0f, "%.2f");

	RE_CameraManager::EditorCamera()->DrawAsEditorProperties();

	ImGui::Separator();
	ImGui::Checkbox("Select on mouse click", &select_on_mc);
	ImGui::Checkbox("Focus on Select", &focus_on_select);
	ImGui::Separator();

	// Debug Drawing
	ImGui::Checkbox("Debug Draw", &debug_drawing);
	if (debug_drawing)
	{
		bool active_grid = grid->IsActive();
		if (ImGui::Checkbox("Draw Grid", &active_grid))
			grid->SetActive(active_grid);

		if (active_grid && ImGui::DragFloat2("Grid Size", grid_size, 0.2f, 0.01f, 100.0f, "%.1f"))
		{
			grid->GetTransformPtr()->SetScale(math::vec(grid_size[0], 0.f, grid_size[1]));
			grid->GetTransformPtr()->Update();
		}

		int aabb_d = static_cast<int>(aabb_drawing);
		if (ImGui::Combo("Draw AABB", &aabb_d, "None\0Selected only\0All\0All w/ different selected\0"))
			aabb_drawing = static_cast<AABBDebugDrawing>(aabb_d);

		if (aabb_drawing > AABBDebugDrawing::SELECTED_ONLY) ImGui::ColorEdit3("Color AABB", all_aabb_color);
		if (static_cast<int>(aabb_drawing) % 2 == 1) ImGui::ColorEdit3("Color Selected", sel_aabb_color);

		ImGui::Checkbox("Draw QuadTree", &draw_quad_tree);
		if (draw_quad_tree) ImGui::ColorEdit3("Color Quadtree", quad_tree_color);

		ImGui::Checkbox("Draw Camera Fustrums", &draw_cameras);
		if (draw_cameras) ImGui::ColorEdit3("Color Fustrum", frustum_color);
	}
}

void ModuleEditor::Load()
{
	RE_PROFILE(PROF_Load, PROF_ModuleWindow);
	RE_LOG_SECONDARY("Loading Editor propieties from config:");

	size_t _size = 0;
	const char* buff = ImGui::SaveIniSettingsToMemory(&_size);
	RE_FileBuffer _load("imgui.ini");
	if (_load.Load())
		ImGui::LoadIniSettingsFromMemory(_load.GetBuffer(), _load.GetSize());

	RE_Json* node = RE_FS->ConfigNode("Editor");

	cam_speed = node->PullFloat("C_Speed", 25.0f);
	cam_sensitivity = node->PullFloat("C_Sensitivity", 0.01f);

	//Editor Camera
	RE_CompCamera* editor_camera = RE_CameraManager::EditorCamera();

	editor_camera->draw_frustum = node->PullBool("C_DrawFrustum", true);
	editor_camera->override_cull = node->PullBool("C_OverrideCull", false);

	math::float2 planes = node->PullFloat("C_Planes", { 1.0f, 5000.0f });
	editor_camera->SetPlanesDistance(planes.x, planes.y);

	editor_camera->target_ar = static_cast<RE_CompCamera::AspectRatioTYPE>(node->PullInt("C_AspectRatio", static_cast<int>(RE_CompCamera::AspectRatioTYPE::Fit_Window)));
	editor_camera->isPerspective = node->PullBool("C_Prespective", true);
	editor_camera->SetFOV(math::RadToDeg(node->PullFloat("C_VerticalFOV", math::DegToRad(30.0f))));
	//------------------

	select_on_mc = node->PullBool("SelectMouseClick", true);
	focus_on_select = node->PullBool("FocusOnSelect", false);

	debug_drawing = node->PullBool("DebugDraw", true);

	grid->SetActive(node->PullBool("Grid_Draw", true));

	math::float2 grid_size_vec = node->PullFloat("Grid_Size", { 1.0f, 1.0f });
	memcpy_s(grid_size, sizeof(float) * 2, grid_size_vec.ptr(), sizeof(float) * 2);
	grid->GetTransformPtr()->SetScale(math::vec(grid_size[0], 0.f, grid_size[1]));
	grid->GetTransformPtr()->Update();

	aabb_drawing = static_cast<AABBDebugDrawing>(node->PullInt("AABB_Drawing", static_cast<int>(AABBDebugDrawing::ALL_AND_SELECTED)));

	math::vec temp_color = node->PullFloatVector("AABB_Selected_Color", { 0.0f, 1.0f, 0.0f });
	memcpy_s(all_aabb_color, sizeof(float) * 3, temp_color.ptr(), sizeof(float) * 3);

	temp_color = node->PullFloatVector("AABB_Color", { 1.0f, 0.5f, 0.0f });
	memcpy_s(sel_aabb_color, sizeof(float) * 3, temp_color.ptr(), sizeof(float) * 3);

	draw_quad_tree = node->PullBool("QuadTree_Draw", true);

	temp_color = node->PullFloatVector("Quadtree_Color", { 1.0f, 1.0f, 0.0f });
	memcpy_s(quad_tree_color, sizeof(float) * 3, temp_color.ptr(), sizeof(float) * 3);

	draw_cameras = node->PullBool("Frustum_Draw", true);

	temp_color = node->PullFloatVector("Frustum_Color", { 0.0f, 1.0f, 1.0f });
	memcpy_s(frustum_color, sizeof(float) * 3, temp_color.ptr(), sizeof(float) * 3);

	DEL(node);
}

void ModuleEditor::Save() const
{
	RE_PROFILE(PROF_Save, PROF_ModuleWindow);
	
	size_t _size = 0;
	const char* buff = ImGui::SaveIniSettingsToMemory(&_size);
	RE_FileBuffer _save("imgui.ini");
	_save.Save(const_cast<char*>(buff), _size);

	RE_Json* node = RE_FS->ConfigNode("Editor");

	node->PushFloat("C_Speed", cam_speed);
	node->PushFloat("C_Sensitivity", cam_sensitivity);

	//Editor Camera
	RE_CompCamera* editor_camera = RE_CameraManager::EditorCamera();
	node->PushBool("C_DrawFrustum", editor_camera->draw_frustum);
	node->PushBool("C_OverrideCull", editor_camera->override_cull);
	node->PushFloat("C_Planes", { editor_camera->GetNearPlane(), editor_camera->GetFarPlane() });
	node->PushInt("C_AspectRatio", static_cast<int>(editor_camera->target_ar));
	node->PushBool("C_Prespective", editor_camera->isPerspective);
	node->PushFloat("C_VerticalFOV", editor_camera->v_fov_rads);
	//------------------

	node->PushBool("SelectMouseClick", select_on_mc);
	node->PushBool("FocusOnSelect", focus_on_select);

	node->PushBool("DebugDraw", debug_drawing);

	node->PushBool("Grid_Draw", grid->IsActive());
	node->PushFloat("Grid_Size", { grid_size[0], grid_size[0]});

	node->PushInt("AABB_Drawing", static_cast<int>(aabb_drawing));

	node->PushFloatVector("AABB_Selected_Color", { all_aabb_color[0], all_aabb_color[1], all_aabb_color[2] });
	node->PushFloatVector("AABB_Color", { sel_aabb_color[0], sel_aabb_color[1], sel_aabb_color[2] });

	node->PushBool("QuadTree_Draw", draw_quad_tree);
	node->PushFloatVector("Quadtree_Color", { quad_tree_color[0], quad_tree_color[1], quad_tree_color[2] });

	node->PushBool("Frustum_Draw", draw_cameras);
	node->PushFloatVector("Frustum_Color", { frustum_color[0], frustum_color[1], frustum_color[2] });

	DEL(node);
}

void ModuleEditor::DrawDebug(RE_CompCamera* current_camera) const
{
	RE_PROFILE(PROF_DrawDebug, PROF_ModuleEditor);

	AABBDebugDrawing adapted_AABBdraw = aabb_drawing;

	if (!selected) adapted_AABBdraw = static_cast<AABBDebugDrawing>(static_cast<int>(adapted_AABBdraw) - 1);
	
	adapted_AABBdraw = (selected ? aabb_drawing : static_cast<AABBDebugDrawing>(static_cast<int>(aabb_drawing) - 1));

	if (debug_drawing && ((adapted_AABBdraw != AABBDebugDrawing::NONE) || draw_quad_tree || draw_cameras))
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(current_camera->GetProjectionPtr());
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf((current_camera->GetView()).ptr());
		glBegin(GL_LINES);

		// Draw Bounding Boxes
		switch (adapted_AABBdraw)
		{
		case AABBDebugDrawing::SELECTED_ONLY:
		{
			glColor4f(sel_aabb_color[0], sel_aabb_color[1], sel_aabb_color[2], 1.0f);
			RE_SCENE->GetGOCPtr(selected)->DrawGlobalAABB();
			break;
		}
		case AABBDebugDrawing::ALL:
		{
			eastl::queue<const RE_GameObject*> objects;
			for (auto child : RE_SCENE->GetRootCPtr()->GetChildsPtr())
				objects.push(child);

			if (!objects.empty())
			{
				glColor4f(all_aabb_color[0], all_aabb_color[1] * 255.0f, all_aabb_color[2], 1.0f);

				const RE_GameObject* object = nullptr;
				while (!objects.empty())
				{
					(object = objects.front())->DrawGlobalAABB();
					objects.pop();

					if (object->ChildCount() > 0u)
						for (auto child : object->GetChildsPtr())
							objects.push(child);
				}
			}

			break;
		}
		case AABBDebugDrawing::ALL_AND_SELECTED:
		{
			glColor4f(sel_aabb_color[0], sel_aabb_color[1], sel_aabb_color[2], 1.0f);
			RE_SCENE->GetGOCPtr(selected)->DrawGlobalAABB();

			eastl::queue<const RE_GameObject*> objects;
			for (auto child : RE_SCENE->GetRootCPtr()->GetChildsPtr())
				objects.push(child);

			if (!objects.empty())
			{
				glColor4f(all_aabb_color[0], all_aabb_color[1], all_aabb_color[2], 1.0f);

				while (!objects.empty())
				{
					const RE_GameObject* object = objects.front();
					objects.pop();

					if (object->GetUID() != selected) object->DrawGlobalAABB();

					if (object->ChildCount() > 0)
						for (auto child : object->GetChildsPtr())
							objects.push(child);
				}
			}

			break;
		}
		}

		if (draw_quad_tree)
		{
			glColor4f(quad_tree_color[0], quad_tree_color[1], quad_tree_color[2], 1.0f);
			RE_SCENE->DrawSpacePartitioning();
		}

		if (draw_cameras)
		{
			glColor4f(frustum_color[0], frustum_color[1], frustum_color[2], 1.0f);
			for (auto cam : RE_SCENE->GetScenePool()->GetAllCompPtr(C_CAMERA))
				static_cast<RE_CompCamera*>(cam)->DrawFrustum();
		}

		glEnd();

		if (grid->IsActive()) grid->Draw();
	}
}

void ModuleEditor::DrawHeriarchy()
{
	GO_UID to_select = 0ull, goToDelete_uid = 0ull;

	const RE_GameObject* root = RE_SCENE->GetRootCPtr();
	GO_UID root_uid = root->GetUID();

	if (root->ChildCount() > 0)
	{
		eastl::stack<RE_GameObject*> gos;
		for (auto child : root->GetChildsPtrReversed()) gos.push(child);

		unsigned int count = 0;
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
	if (goToDelete_uid != 0ull) {
		RE_INPUT->Push(RE_EventType::DESTROY_GO, RE_SCENE, goToDelete_uid);
		if (selected == goToDelete_uid || RE_SCENE->GetGOPtr(goToDelete_uid)->isParent(selected)) selected = 0ull;
	}
}

GO_UID ModuleEditor::GetSelected() const { return selected; }

void ModuleEditor::SetSelected(const GO_UID go, bool force_focus)
{
	selected = go;
	RE_RES->PopSelected(true);
	if (force_focus || (focus_on_select && selected))
	{
		math::AABB box = RE_SCENE->GetGOCPtr(selected)->GetGlobalBoundingBoxWithChilds();
		RE_CameraManager::EditorCamera()->Focus(box.CenterPoint(), box.HalfSize().Length());
	}
}

void ModuleEditor::DuplicateSelectedObject()
{
	if (selected)
	{
		const RE_GameObject* sel_go = RE_SCENE->GetGOCPtr(selected);
		RE_SCENE->GetGOPtr(RE_SCENE->GetScenePool()->CopyGOandChilds(sel_go, sel_go->GetParentUID(), true))->UseResources();
	}
}

void ModuleEditor::ReportSoftawe(const char* name, const char* version, const char* website) const
{
	about->sw_info.push_back({ name, version, website });
}

void ModuleEditor::HandleSDLEvent(SDL_Event* e) { ImGui_ImplSDL2_ProcessEvent(e); }
void ModuleEditor::PopUpFocus(bool focus) { popUpFocus = focus; }
const char* ModuleEditor::GetAssetsPanelPath() const { return assets->GetCurrentDirPath(); }
void ModuleEditor::SelectUndefinedFile(eastl::string* toSelect) const { assets->SelectUndefined(toSelect); }

void ModuleEditor::OpenTextEditor(const char* filePath, eastl::string* filePathStr, const char* shadertTemplate, bool* open)
{
	texteditormanager->PushEditor(filePath, filePathStr, shadertTemplate, open);
}

void ModuleEditor::GetSceneWindowSize(unsigned int* widht, unsigned int* height)
{
	*widht = sceneEditorWindow->GetSceneWidth();
	*height = sceneEditorWindow->GetSceneHeight();
}

void ModuleEditor::StartEditingParticleEmitter(RE_ParticleEmitter* sim, const char* md5)
{
	particleEmitterWindow->StartEditing(sim, md5);
}

const RE_ParticleEmitter* ModuleEditor::GetCurrentEditingParticleEmitter() const
{
	return particleEmitterWindow->GetEdittingParticleEmitter();
}

void ModuleEditor::SaveEmitter(bool close, const char* emitter_name, const char* emissor_base, const char* renderer_base)
{
	particleEmitterWindow->SaveEmitter(close, emitter_name, emissor_base, renderer_base);
}

void ModuleEditor::CloseParticleEditor()
{
	particleEmitterWindow->NextOrClose();
}

bool ModuleEditor::IsParticleEditorActive() const { return particleEmitterWindow->IsActive(); }

bool ModuleEditor::EditorSceneNeedsRender() const
{
	return sceneEditorWindow->NeedRender();
}

bool ModuleEditor::GameSceneNeedsRender() const
{
	return sceneGameWindow->NeedRender();
}

void ModuleEditor::PushCommand(RE_Command* cmd) { commands->PushCommand(cmd); }
void ModuleEditor::ClearCommands() { commands->Clear(); }

void ModuleEditor::UpdateCamera()
{
	RE_PROFILE(PROF_EditorCamera, PROF_ModuleEditor);
	RE_CompCamera* camera = RE_CameraManager::EditorCamera();
	if (sceneEditorWindow->isSelected())
	{
		ImVec2 mouseData = ImGui::GetIO().MouseDelta;

		if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_LeftAlt)->Down && ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Left))
		{
			// Orbit
			if (selected && (mouseData.x || mouseData.y))
				camera->Orbit(cam_sensitivity * -mouseData.x, cam_sensitivity * mouseData.y, RE_SCENE->GetGOCPtr(selected)->GetGlobalBoundingBoxWithChilds().CenterPoint());
		}
		else if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_F)->Down && selected)
		{
			// Focus
			math::AABB box = RE_SCENE->GetGOCPtr(selected)->GetGlobalBoundingBoxWithChilds();
			camera->Focus(box.CenterPoint(), box.HalfSize().Length());
		}
		else
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Right))
			{
				// Camera Speed
				float cameraSpeed = cam_speed * RE_TIME->GetDeltaTime();
				if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_LeftShift)->Down) cameraSpeed *= 2.0f;
				
				// Move
				if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_W)->Down)			camera->LocalMove(Dir::FORWARD, cameraSpeed);
				if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_S)->Down)			camera->LocalMove(Dir::BACKWARD, cameraSpeed);
				if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_A)->Down)			camera->LocalMove(Dir::LEFT, cameraSpeed);
				if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_D)->Down)			camera->LocalMove(Dir::RIGHT, cameraSpeed);
				if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_Space)->Down)		camera->LocalMove(Dir::UP, cameraSpeed);
				if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_C)->Down)			camera->LocalMove(Dir::DOWN, cameraSpeed);

				// Rotate
				if (mouseData.x != 0 || mouseData.y != 0)
					camera->LocalPan(cam_sensitivity * -mouseData.x, cam_sensitivity * mouseData.y);
			}

			float mouseWheelData = ImGui::GetIO().MouseWheel;

			// Zoom
			if (mouseWheelData != 0) camera->SetFOV(camera->GetVFOVDegrees() - mouseWheelData);
		}
	}

	camera->Update();

	if (particleEmitterWindow->IsActive()) {
		camera = RE_CameraManager::ParticleEditorCamera();

		if (particleEmitterWindow->isSelected()) {
			ImVec2 mouseData = ImGui::GetIO().MouseDelta;

			if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_F)->Down)
			{
				// Focus
				math::AABB box = particleEmitterWindow->GetEdittingParticleEmitter()->bounding_box;
				camera->Focus(box.CenterPoint(), box.HalfSize().Length());
			}
			else
			{
				if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Right))
				{
					// Camera Speed
					float cameraSpeed = cam_speed * RE_TIME->GetDeltaTime();
					if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_LeftShift)->Down) cameraSpeed *= 2.0f;

					// Move
					if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_W)->Down)			camera->LocalMove(Dir::FORWARD, cameraSpeed);
					if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_S)->Down)			camera->LocalMove(Dir::BACKWARD, cameraSpeed);
					if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_A)->Down)			camera->LocalMove(Dir::LEFT, cameraSpeed);
					if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_D)->Down)			camera->LocalMove(Dir::RIGHT, cameraSpeed);
					if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_Space)->Down)		camera->LocalMove(Dir::UP, cameraSpeed);
					if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_C)->Down)			camera->LocalMove(Dir::DOWN, cameraSpeed);

					// Rotate
					if (mouseData.x != 0 || mouseData.y != 0)
						camera->LocalPan(cam_sensitivity * -mouseData.x, cam_sensitivity * mouseData.y);
				}

				float mouseWheelData = ImGui::GetIO().MouseWheel;
				// Zoom
				if (mouseWheelData != 0) camera->SetFOV(camera->GetVFOVDegrees() - mouseWheelData);
			}
		}

		camera->Update();
	}
}

void ModuleEditor::DrawGameObjectItems(const GO_UID parent)
{
	if (ImGui::BeginMenu("Primitive"))
	{
		if (ImGui::MenuItem("Grid")) RE_SCENE->CreatePrimitive(C_GRID, parent);
		if (ImGui::MenuItem("Cube")) RE_SCENE->CreatePrimitive(C_CUBE, parent);
		if (ImGui::MenuItem("Dodecahedron")) RE_SCENE->CreatePrimitive(C_DODECAHEDRON, parent);
		if (ImGui::MenuItem("Tetrahedron")) RE_SCENE->CreatePrimitive(C_TETRAHEDRON, parent);
		if (ImGui::MenuItem("Octohedron")) RE_SCENE->CreatePrimitive(C_OCTOHEDRON, parent);
		if (ImGui::MenuItem("Icosahedron")) RE_SCENE->CreatePrimitive(C_ICOSAHEDRON, parent);
		if (ImGui::MenuItem("Point")) RE_SCENE->CreatePrimitive(C_POINT, parent);
		if (ImGui::MenuItem("Plane")) RE_SCENE->CreatePrimitive(C_PLANE, parent);
		// if (ImGui::MenuItem("Frustum")) RE_SCENE->CreatePrimitive(C_FUSTRUM, parent);
		if (ImGui::MenuItem("Sphere")) RE_SCENE->CreatePrimitive(C_SPHERE, parent);
		if (ImGui::MenuItem("Cylinder")) RE_SCENE->CreatePrimitive(C_CYLINDER, parent);
		if (ImGui::MenuItem("HemiSphere")) RE_SCENE->CreatePrimitive(C_HEMISHPERE, parent);
		if (ImGui::MenuItem("Torus")) RE_SCENE->CreatePrimitive(C_TORUS, parent);
		if (ImGui::MenuItem("Trefoil Knot")) RE_SCENE->CreatePrimitive(C_TREFOILKNOT, parent);
		if (ImGui::MenuItem("Rock")) RE_SCENE->CreatePrimitive(C_ROCK, parent);

		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("Camera")) RE_SCENE->CreateCamera(parent);
	if (ImGui::MenuItem("Light")) RE_SCENE->CreateLight(parent);
	if (ImGui::MenuItem("Max Lights")) RE_SCENE->CreateMaxLights(parent);
	if (ImGui::MenuItem("Water")) RE_SCENE->CreateWater(parent);
	if (ImGui::MenuItem("Particle System")) RE_SCENE->CreateParticleSystem(parent);
}
