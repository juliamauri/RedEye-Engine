#include "ModuleEditor.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "EditorWindows.h"
#include "TimeManager.h"
#include "OutputLog.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"

#include "MathGeoLib\include\MathGeoLib.h"
#include "ImGui\imgui_impl_sdl_gl3.h"
#include "ImGuizmo\ImGuizmo.h"
#include "glew\include\glew.h"
#include "SDL2\include\SDL.h"

ModuleEditor::ModuleEditor(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	windows.push_back(console = new ConsoleWindow());
	windows.push_back(config = new ConfigWindow());
	windows.push_back(heriarchy = new HeriarchyWindow());
	windows.push_back(properties = new PropertiesWindow());
	about = new AboutWindow();

	tools.push_back(rng = new RandomTest());
	tools.push_back(textures = new TexturesWindow());
}

ModuleEditor::~ModuleEditor()
{
	windows.clear();
	tools.clear();

	DEL(camera);
}

// Load assets
bool ModuleEditor::Init(JSONNode* node)
{
	LOG_SECONDARY("Init ImGui");

	bool ret = ImGui_ImplSdlGL3_Init(App->window->GetWindow());

	if (ret)
		App->ReportSoftware("ImGui", IMGUI_VERSION, "https://github.com/ocornut/imgui");
	else
		LOG_ERROR("ImGui could not initialize!");

	// TODO set window lock positions


	camera = new RE_CompCamera();

	return ret;

}

update_status ModuleEditor::PreUpdate()
{
	ImGui_ImplSdlGL3_NewFrame(App->window->GetWindow());
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

				// tools submenu
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

		// Windows
		for (auto window : windows)
		{
			if (window->IsActive())
				window->DrawWindow();
		}

		if (about != nullptr && about->IsActive())
			about->DrawWindow(); // (not in windows' list)

		for (auto tool : tools)
		{
			if (tool->IsActive())
				tool->DrawWindow();
		}

	}
	
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

	DEL(about);

	ImGui_ImplSdlGL3_Shutdown();

	return true;
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
}

void ModuleEditor::HandleSDLEvent(SDL_Event* e)
{
	ImGui_ImplSdlGL3_ProcessEvent(e);
}

RE_CompCamera * ModuleEditor::GetCamera() const
{
	return camera;
}

void ModuleEditor::UpdateCamera()
{

	if (!ImGui::IsMouseHoveringAnyWindow())
	{
		const MouseData* mouse = App->input->GetMouse();
		RE_CompTransform* transform = camera->GetTransform();

		if (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT && mouse->GetButton(1) == KEY_REPEAT)
		{
			if (App->scene->GetSelected() != nullptr
				&& (mouse->mouse_x_motion || mouse->mouse_y_motion))
			{
				transform->Orbit(
					-mouse->mouse_x_motion,
					mouse->mouse_y_motion,
					App->scene->GetSelected()->GetBoundingBox().CenterPoint());
			}
		}
		else if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN
			&& App->scene->GetSelected() != nullptr)
		{
			transform->SetPos(App->scene->GetSelected()->GetBoundingBox().maxPoint * 2);
			transform->LocalLookAt(App->scene->GetSelected()->GetBoundingBox().CenterPoint());
		}
		else
		{
			if (mouse->GetButton(3) == KEY_REPEAT)
			{
				float cameraSpeed = 5.f;
				if (App->input->CheckKey(SDL_SCANCODE_LSHIFT, KEY_REPEAT))
					cameraSpeed *= 2.0f;
				cameraSpeed *= App->time->GetDeltaTime();

				if (App->input->CheckKey(SDL_SCANCODE_W, KEY_REPEAT))
					transform->LocalMove(Dir::FORWARD, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_S, KEY_REPEAT))
					transform->LocalMove(Dir::BACKWARD, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_A, KEY_REPEAT))
					transform->LocalMove(Dir::LEFT, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_D, KEY_REPEAT))
					transform->LocalMove(Dir::RIGHT, cameraSpeed);

				camera->RotateWithMouse(mouse->mouse_x_motion, -mouse->mouse_y_motion);
			}

			camera->MouseWheelZoom(mouse->mouse_wheel_motion);
		}

		camera->Update();
	}
}