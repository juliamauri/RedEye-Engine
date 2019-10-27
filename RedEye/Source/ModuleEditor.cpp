#include "ModuleEditor.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleRenderer3D.h"
#include "EditorWindows.h"
#include "TimeManager.h"
#include "OutputLog.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"

#include "MathGeoLib\include\MathGeoLib.h"
#include "ImGui\imgui_impl_opengl3.h"
#include "ImGui\imgui_impl_sdl.h"
#include "ImGuizmo\ImGuizmo.h"
#include "ImGui/imgui_internal.h"
#include "glew\include\glew.h"
#include "SDL2\include\SDL.h"

ModuleEditor::ModuleEditor(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	popupWindow = new PopUpWindow();
	windows.push_back(console = new ConsoleWindow());
	windows.push_back(config = new ConfigWindow());
	windows.push_back(heriarchy = new HeriarchyWindow());
	windows.push_back(properties = new PropertiesWindow());
	windows.push_back(editor_settings = new EditorSettingsWindow());
	windows.push_back(play_pause = new PlayPauseWindow());
	windows.push_back(select_file = new SelectFile());
	windows.push_back(prefabsPanel = new PrefabsPanel());
	about = new AboutWindow();

	tools.push_back(rng = new RandomTest());
	tools.push_back(textures = new TexturesWindow());
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

	// Editor camera
	camera = new RE_CompCamera();
	camera->GetTransform()->SetPosition(math::vec(0.f, 5.f, -5.f));

	// ImGui
	LOG_SECONDARY("Init ImGui");
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	if (ret = ImGui_ImplSDL2_InitForOpenGL(App->window->GetWindow(), App->renderer3d->GetContext()))
	{
		if (ret = ImGui_ImplOpenGL3_Init())
			App->ReportSoftware("ImGui", IMGUI_VERSION, "https://github.com/ocornut/imgui");
		else
			LOG_ERROR("ImGui could not OpenGL3_Init!");
	}
	else
		LOG_ERROR("ImGui could not SDL2_InitForOpenGL!");

	return ret;
}

update_status ModuleEditor::PreUpdate()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->GetWindow());
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
	return UPDATE_CONTINUE;
}


// Update
update_status ModuleEditor::Update()
{
	if (show_all)
	{
		// Main Menu Bar
		if (ImGui::BeginMainMenuBar())
		{
			// File
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem(" Exit", "	Esc"))
				{
					App->input->AddEvent(Event(REQUEST_QUIT, App));
				}
				ImGui::EndMenu();
			}

			// Create
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Cube"))
					App->scene->CreateCube();

				if (ImGui::MenuItem("Sphere"))
					App->scene->CreateSphere();

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
	DEL(camera);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return true;
}

void ModuleEditor::DrawEditor()
{
	if (ImGui::CollapsingHeader(GetName()))
		ImGui::DragFloat("Focus min dist", &min_focus_dist, 0.1f, 0.f, 50.f, "%.1f");
}

void ModuleEditor::FocusSelected()
{
	camera->Focus(App->scene->GetSelected(), min_focus_dist);
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
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ModuleEditor::HandleSDLEvent(SDL_Event* e)
{
	ImGui_ImplSDL2_ProcessEvent(e);
}

RE_CompCamera * ModuleEditor::GetCamera() const
{
	return camera;
}

SelectFile * ModuleEditor::GetSelectWindow()
{
	return select_file;
}

void ModuleEditor::PopUpFocus(bool focus)
{
	popUpFocus = focus;
}

void ModuleEditor::UpdateCamera()
{
	if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
	{
		const MouseData* mouse = App->input->GetMouse();

		if (mouse->GetButton(1) == KEY_DOWN)
		{
			// Mouse Pick
			App->scene->RayCastSelect(
				math::Ray(camera->GetFrustum().UnProjectLineSegment(
				(2.f * mouse->mouse_x) / App->window->GetWidth() - 1.f,
					1.f - (2.f * mouse->mouse_y) / App->window->GetHeight())));
		}
		else if (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT && mouse->GetButton(1) == KEY_REPEAT)
		{
			// Orbit
			if (App->scene->GetSelected() != nullptr
				&& (mouse->mouse_x_motion || mouse->mouse_y_motion))
			{
				camera->Orbit(
					CAM_SENSITIVITY * -mouse->mouse_x_motion,
					CAM_SENSITIVITY * mouse->mouse_y_motion,
					App->scene->GetSelected());
			}
		}
		else if ((App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN) && App->scene->GetSelected() != nullptr)
		{
			// Focus
			FocusSelected();
		}
		else
		{
			if (mouse->GetButton(3) == KEY_REPEAT)
			{
				// Camera Speed
				float cameraSpeed = CAM_SPEED * App->time->GetDeltaTime();
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
				if (mouse->mouse_x_motion != 0 || mouse->mouse_y_motion != 0)
				{
					camera->LocalRotate(
						CAM_SENSITIVITY * -mouse->mouse_x_motion,
						CAM_SENSITIVITY * mouse->mouse_y_motion);
				}
			}

			// Zoom
			if (mouse->mouse_wheel_motion != 0)
				camera->SetVerticalFOV(camera->GetVFOVDegrees() - mouse->mouse_wheel_motion);
		}

		camera->Update();
	}
}
