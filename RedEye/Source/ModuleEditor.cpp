 #include "ModuleEditor.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "RE_ResourceManager.h"
#include "ModuleRenderer3D.h"
#include "EditorWindows.h"
#include "TimeManager.h"
#include "OutputLog.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_CameraManager.h"
#include "RE_PrimitiveManager.h"
#include "QuadTree.h"
#include "RE_GLCache.h"

#include "MathGeoLib\include\MathGeoLib.h"
#include "ImGui\imgui_impl_opengl3.h"
#include "ImGui\imgui_impl_sdl.h"
#include "ImGuizmo\ImGuizmo.h"
#include "ImGui/imgui_internal.h"
#include "glew\include\glew.h"
#include "SDL2\include\SDL.h"

#include <stack>

ModuleEditor::ModuleEditor(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	popupWindow = new PopUpWindow();
	windows.push_back(console = new ConsoleWindow());
	windows.push_back(config = new ConfigWindow());
	windows.push_back(heriarchy = new HeriarchyWindow());
	windows.push_back(properties = new PropertiesWindow());
	windows.push_back(play_pause = new PlayPauseWindow());
	about = new AboutWindow();
	select_file = new SelectFile();

	tools.push_back(rng = new RandomTest());
	tools.push_back(textures = new TexturesWindow());
	tools.push_back(materialeditor = new MaterialEditorWindow());
	tools.push_back(shadereditor = new ShaderEditorWindow());

	grid_size[0] = grid_size[1] = 1.0f;
}

ModuleEditor::~ModuleEditor()
{
	windows.clear();
	tools.clear();
}

// Load assets
bool ModuleEditor::Init(JSONNode* node)
{
	bool ret = true;

	// ImGui
	LOG_SECONDARY("Init ImGui");
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	if (ret = ImGui_ImplSDL2_InitForOpenGL(App->window->GetWindow(), App->renderer3d->GetWindowContext()))
	{
		if (ret = ImGui_ImplOpenGL3_Init())
			App->ReportSoftware("ImGui", IMGUI_VERSION, "https://github.com/ocornut/imgui");
		else
			LOG_ERROR("ImGui could not OpenGL3_Init!");
	}
	else
		LOG_ERROR("ImGui could not SDL2_InitForOpenGL!");


	all_aabb_color[0] = 0.f;
	all_aabb_color[1] = 1.f;
	all_aabb_color[2] = 0.f;

	sel_aabb_color[0] = 1.f;
	sel_aabb_color[1] = 1.f;
	sel_aabb_color[2] = 1.f;

	quad_tree_color[0] = 1.f;
	quad_tree_color[1] = 1.f;
	quad_tree_color[2] = 0.f;

	frustum_color[0] = 0.f;
	frustum_color[1] = 1.f;
	frustum_color[2] = 1.f;

	return ret;
}

bool ModuleEditor::Start()
{
	windows.push_back(assets = new AssetsWindow());

	grid_go = new RE_GameObject("grid");
	grid = (RE_Component*)App->primitives->CreateGrid(grid_go);

	// FOCUS CAMERA
	const RE_GameObject* root = App->scene->GetRoot();
	if (!root->GetChilds().empty()) {
		SetSelected(root->GetChilds().begin()._Ptr->_Myval);
		RE_CameraManager::EditorCamera()->Focus(selected);
	}

	return true;
}

update_status ModuleEditor::PreUpdate()
{
	OPTICK_CATEGORY("PreUpdate ModuleEditor", Optick::Category::GameLogic);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->GetWindow());
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
	return UPDATE_CONTINUE;
}


// Update
update_status ModuleEditor::Update()
{
	OPTICK_CATEGORY("Update ModuleEditor", Optick::Category::UI);
	if (show_all)
	{
		// Main Menu Bar
		if (ImGui::BeginMainMenuBar())
		{
			// File
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem(" Exit", "	Esc"))
					Event::Push(REQUEST_QUIT, App);

				ImGui::EndMenu();
			}

			// Create
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Plane"))
					App->scene->CreatePlane();

				if (ImGui::MenuItem("Cube"))
					App->scene->CreateCube();

				if (ImGui::MenuItem("Sphere"))
					App->scene->CreateSphere();

				if (ImGui::MenuItem("Camera"))
					App->scene->CreateCamera();

				ImGui::EndMenu();
			}

			// View
			if (ImGui::BeginMenu("View"))
			{
				for (auto window : windows)
				{
					if (ImGui::MenuItem(window->Name(), window->IsActive() ? "Hide" : "Open"))
					{
						window->SwitchActive();
					}
				}

				// Tools submenu
				if (ImGui::BeginMenu("Tools"))
				{
					for (auto tool : tools)
					{
						if (ImGui::MenuItem(tool->Name(), tool->IsActive() ? "Hide" : "Open"))
						{
							tool->SwitchActive();
						}
					}

					ImGui::EndMenu();
				}

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
					about->SwitchActive();

				ImGui::EndMenu();
			}

			if (show_demo) ImGui::ShowTestWindow();

			ImGui::EndMainMenuBar();
		}

		if (popupWindow->IsActive()) popupWindow->DrawWindow();

		if (select_file->IsActive()) select_file->DrawWindow();

		// Draw Windows
		for (auto window : windows)
		{
			if (window->IsActive())
				window->DrawWindow(popUpFocus);
		}

		if (about != nullptr && about->IsActive())
			about->DrawWindow(popUpFocus); // (not in windows' list)

		for (auto tool : tools)
		{
			if (tool->IsActive())
				tool->DrawWindow(popUpFocus);
		}
	}
	
	// Toggle show editor on F1
	if(App->input->CheckKey(SDL_SCANCODE_F1))
		show_all = !show_all;

	// CAMERA CONTROLS
	if (App->GetState() == GS_STOP)
		UpdateCamera();

	return UPDATE_CONTINUE;
}

// Load assets
bool ModuleEditor::CleanUp()
{
	while (!windows.empty())
	{
		delete *(windows.rbegin());
		windows.pop_back();
	}

	while (!tools.empty())
	{
		delete *(tools.rbegin());
		tools.pop_back();
	}

	DEL(popupWindow);
	DEL(about);
	DEL(select_file);

	DEL(grid_go);
	grid = nullptr;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return true;
}

void ModuleEditor::DrawEditor()
{
	if (ImGui::CollapsingHeader(GetName()))
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
				grid_go->GetTransform()->SetScale(math::vec(grid_size[0], 0.f, grid_size[1]));
				grid_go->GetTransform()->Update();
			}

			int aabb_d = aabb_drawing;
			if (ImGui::Combo("Draw AABB", &aabb_d, "None\0Selected only\0All\0All w/ different selected\0"))
				aabb_drawing = AABBDebugDrawing(aabb_d);
			
			if (aabb_drawing > SELECTED_ONLY) ImGui::ColorEdit3("Color AABB", all_aabb_color);
			if (aabb_drawing%2 == 1) ImGui::ColorEdit3("Color Selected", sel_aabb_color);

			ImGui::Checkbox("Draw QuadTree", &draw_quad_tree);
			if (draw_quad_tree) ImGui::ColorEdit3("Color Quadtree", quad_tree_color);

			ImGui::Checkbox("Draw Camera Fustrums", &draw_cameras);
			if (draw_cameras) ImGui::ColorEdit3("Color Fustrum", frustum_color);
		}
	}
}

void ModuleEditor::DrawDebug(bool resetLight) const
{
	OPTICK_CATEGORY("Debug Draw", Optick::Category::Debug);

	AABBDebugDrawing adapted_AABBdraw = (selected != nullptr ? aabb_drawing : AABBDebugDrawing(aabb_drawing - 1));

	// Draw Bounding Boxes
	if (debug_drawing && ((adapted_AABBdraw != AABBDebugDrawing::NONE) || draw_quad_tree || draw_cameras))
	{
		RE_GLCache::ChangeShader(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		const RE_GameObject* root = App->scene->GetRoot();
		RE_CompCamera* current_camera = RE_CameraManager::CurrentCamera();

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(current_camera->GetProjectionPtr());
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf((current_camera->GetView()).ptr());

		if (resetLight)
			glDisable(GL_LIGHTING);

		glBegin(GL_LINES);

		switch (adapted_AABBdraw)
		{
		case SELECTED_ONLY:
		{
			glColor3f(sel_aabb_color[0], sel_aabb_color[1], sel_aabb_color[2]);
			selected->DrawGlobalAABB();
			break;
		}
		case ALL:
		{
			std::queue<const RE_GameObject*> objects;
			for (auto child : root->GetChilds())
				objects.push(child);

			if (!objects.empty())
			{
				const RE_GameObject* object = nullptr;

				glColor3f(all_aabb_color[0], all_aabb_color[1], all_aabb_color[2]);

				while (!objects.empty())
				{
					object = objects.front();
					object->DrawGlobalAABB();
					objects.pop();

					if (object->ChildCount() > 0)
						for (auto child : object->GetChilds())
							objects.push(child);
				}
			}

			break;
		}
		case ALL_AND_SELECTED:
		{
			glColor3f(sel_aabb_color[0], sel_aabb_color[1], sel_aabb_color[2]);
			selected->DrawGlobalAABB();

			std::queue<const RE_GameObject*> objects;
			for (auto child : root->GetChilds())
				objects.push(child);

			if (!objects.empty())
			{
				glColor3f(all_aabb_color[0], all_aabb_color[1], all_aabb_color[2]);

				const RE_GameObject* object = nullptr;
				while (!objects.empty())
				{
					object = objects.front();
					objects.pop();

					if (object != selected)
						object->DrawGlobalAABB();

					if (object->ChildCount() > 0)
						for (auto child : object->GetChilds())
							objects.push(child);
				}
			}

			break;
		}
		}

		if (draw_quad_tree)
		{
			glColor3f(quad_tree_color[0], quad_tree_color[1], quad_tree_color[2]);
			App->scene->DrawQTree();
		}

		if (draw_cameras)
		{
			glColor3f(frustum_color[0], frustum_color[1], frustum_color[2]);
			for (auto cam : App->cams->GetCameras())
				cam->DrawFrustum();
		}

		glEnd();

		if (resetLight)
			glEnable(GL_LIGHTING);

		if (grid->IsActive())
			grid->Draw();
	}
}

void ModuleEditor::DrawHeriarchy()
{
	RE_GameObject* to_select = nullptr;
	const RE_GameObject* root = App->scene->GetRoot_c();
	if (root->ChildCount() > 0)
	{
		// Add root children
		std::stack<RE_GameObject*> objects;
		std::list<RE_GameObject*> childs;
		childs = root->GetChilds();
		for (std::list<RE_GameObject*>::reverse_iterator it = childs.rbegin(); it != childs.rend(); it++)
			objects.push(*it);

		bool is_leaf;
		while (!objects.empty())
		{
			RE_GameObject* object = objects.top();
			objects.pop();
			childs.clear();
			childs = object->GetChilds();
			is_leaf = childs.empty();

			if (ImGui::TreeNodeEx(object->GetName(), ImGuiTreeNodeFlags_(selected == object ?
				(is_leaf ? ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_Leaf :
				ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick) :
				(is_leaf ? ImGuiTreeNodeFlags_Leaf :
					ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick))))
			{
				if (is_leaf)
					ImGui::TreePop();
				else
					for (std::list<RE_GameObject*>::reverse_iterator it = childs.rbegin(); it != childs.rend(); it++)
						objects.push(*it);
			}

			if (ImGui::IsItemClicked(0))
				to_select = object;

			if (object->IsLastChild() && object->GetParent_c() != root)
				ImGui::TreePop();
		}
	}

	if (to_select != nullptr)
		SetSelected(to_select);
}

RE_GameObject * ModuleEditor::GetSelected() const
{
	return selected;
}

void ModuleEditor::SetSelected(RE_GameObject* go, bool force_focus)
{
	selected = go;
	App->resources->PopSelected(true);

	if (force_focus || (focus_on_select && selected != nullptr))
		RE_CameraManager::CurrentCamera()->Focus(selected);
}

void ModuleEditor::DuplicateSelectedObject()
{
	if (selected != nullptr)
		selected->GetParent()->AddChild(new RE_GameObject(*selected));

}

void ModuleEditor::LogToEditorConsole()
{
	if (console != nullptr && !windows.empty()
		&& (console->file_filter < 0 || App->log->logHistory.back().caller_id == console->file_filter)
		&& console->categories[(int)App->log->logHistory.back().category])
	{
		console->console_buffer.append(App->log->logHistory.back().data.c_str());
		console->scroll_to_bot = true;
	}
}

bool ModuleEditor::AddSoftwareUsed(const char * name, const char * version, const char * website)
{
	bool ret = (about != nullptr);

	if (ret) about->sw_info.push_back(SoftwareInfo(name, version, website));

	return ret;
}

void ModuleEditor::Draw()
{
	OPTICK_CATEGORY("ImGui Rend", Optick::Category::Rendering);
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ModuleEditor::HandleSDLEvent(SDL_Event* e)
{
	ImGui_ImplSDL2_ProcessEvent(e);
}

SelectFile * ModuleEditor::GetSelectWindow()const
{
	return select_file;
}

void ModuleEditor::PopUpFocus(bool focus)
{
	popUpFocus = focus;
}

const char* ModuleEditor::GetAssetsPanelPath() const
{
	return assets->GetCurrentDirPath();
}

void ModuleEditor::SelectUndefinedFile(std::string* toSelect) const
{
	assets->SelectUndefined(toSelect);
}

void ModuleEditor::UpdateCamera()
{
	OPTICK_CATEGORY("Update ModuleEditor Camera", Optick::Category::Camera);
	RE_CompCamera* camera = RE_CameraManager::EditorCamera();
	if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow))
	{
		const MouseData mouse = App->input->GetMouse();

		if (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_IDLE && mouse.GetButton(1) == KEY_DOWN)
		{
			// Mouse Pick
			int width, height;
			camera->GetTargetWidthHeight(width, height);

			OPTICK_CATEGORY("Update ModuleEditor Camera RayCast", Optick::Category::Camera);
			RE_GameObject* hit = App->scene->RayCastSelect(
				math::Ray(camera->GetFrustum().UnProjectLineSegment(
				(mouse.mouse_x - (width / 2.0f)) / (width / 2.0f),
					((height - mouse.mouse_y) - (height / 2.0f)) / (height / 2.0f))));

			if (hit != nullptr)
				SetSelected(hit);
		}
		else if (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT && mouse.GetButton(1) == KEY_REPEAT)
		{
			// Orbit
			if (selected != nullptr
				&& (mouse.mouse_x_motion || mouse.mouse_y_motion))
			{
				camera->Orbit(
					cam_sensitivity * -mouse.mouse_x_motion,
					cam_sensitivity * mouse.mouse_y_motion,
					*selected);
			}
		}
		else if ((App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN) && selected != nullptr)
		{
			// Focus
			camera->Focus(selected);
		}
		else
		{
			if (mouse.GetButton(3) == KEY_REPEAT)
			{
				// Camera Speed
				float cameraSpeed = cam_speed * App->time->GetDeltaTime();
				if (App->input->CheckKey(SDL_SCANCODE_LSHIFT, KEY_REPEAT))
					cameraSpeed *= 2.0f;

				// Move
				if (App->input->CheckKey(SDL_SCANCODE_W, KEY_REPEAT))
					camera->LocalMove(Dir::FORWARD, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_S, KEY_REPEAT))
					camera->LocalMove(Dir::BACKWARD, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_A, KEY_REPEAT))
					camera->LocalMove(Dir::LEFT, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_D, KEY_REPEAT))
					camera->LocalMove(Dir::RIGHT, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_SPACE, KEY_REPEAT))
					camera->LocalMove(Dir::UP, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_C, KEY_REPEAT))
					camera->LocalMove(Dir::DOWN, cameraSpeed);

				// Rotate
				if (mouse.mouse_x_motion != 0 || mouse.mouse_y_motion != 0)
				{
					camera->LocalRotate(
						cam_sensitivity * -mouse.mouse_x_motion,
						cam_sensitivity * mouse.mouse_y_motion);
				}
			}

			// Zoom
			if (mouse.mouse_wheel_motion != 0)
				camera->SetFOV(camera->GetVFOVDegrees() - mouse.mouse_wheel_motion);
		}
	}

	camera->Update();
}