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
	windows.push_back(editor_settings = new EditorSettingsWindow());
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
	DEL(camera);

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
				// Orbit
				transform->Orbit(
					-mouse->mouse_x_motion,
					mouse->mouse_y_motion,
					App->scene->GetSelected()->GetGlobalBoundingBox().CenterPoint());
			}
		}
		else if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN
			&& App->scene->GetSelected() != nullptr)
		{
			// Focus
			math::AABB selected_box = App->scene->GetSelected()->GetGlobalBoundingBox();
			math::vec target_global_pos = App->scene->GetSelected()->GetTransform()->GetGlobalPosition();
			math::vec camera_pos = selected_box.maxPoint + target_global_pos;

			transform->SetPos(camera_pos);
			transform->LocalLookAt(selected_box.CenterPoint() + target_global_pos);
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
					transform->LocalMove(Dir::FORWARD, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_S, KEY_REPEAT))
					transform->LocalMove(Dir::BACKWARD, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_A, KEY_REPEAT))
					transform->LocalMove(Dir::LEFT, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_D, KEY_REPEAT))
					transform->LocalMove(Dir::RIGHT, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_SPACE, KEY_REPEAT))
					transform->LocalMove(Dir::UP, cameraSpeed);
				if (App->input->CheckKey(SDL_SCANCODE_C, KEY_REPEAT))
					transform->LocalMove(Dir::DOWN, cameraSpeed);

				if (mouse->mouse_x_motion != 0)
					transform->SetRot(math::Quat::RotateY(-1.00f * CAM_SENSITIVITY * DEGTORAD * mouse->mouse_x_motion) * transform->GetLocalRot());

				if (mouse->mouse_y_motion != 0)
				{
					math::Quat desired_rot = transform->GetLocalRot() * math::Quat::RotateX(-1.00f * CAM_SENSITIVITY * DEGTORAD * mouse->mouse_y_motion);
					transform->SetRot(desired_rot);

					// X cap not working
					/*float des_x_euler = math::RadToDeg(desired_rot.ToEulerXYZ()).x;

					if (des_x_euler < 90.f && des_x_euler > -90.f)
						transform->SetRot(desired_rot);
					else
						int blocked = 0;*/
				}
			}

			// Zoom
			if (mouse->mouse_wheel_motion != 0)
				camera->SetVerticalFOV(camera->GetVFOVDegrees() - mouse->mouse_wheel_motion);
		}

		camera->Update();
	}
}