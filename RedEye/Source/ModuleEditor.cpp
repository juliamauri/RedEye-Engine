#include "ModuleEditor.h"

#include "Application.h"
#include "ModuleWindow.h"

#include "ImGui\imgui_impl_sdl_gl3.h"
#include "glew\include\glew.h"
#include "SDL2\include\SDL.h"

#include "ImGuizmo\ImGuizmo.h"

ModuleEditor::ModuleEditor(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	
}

ModuleEditor::~ModuleEditor()
{

}

// Load assets
bool ModuleEditor::Start()
{
	bool ret = ImGui_ImplSdlGL3_Init(App->window->GetWindow());
	
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
	// Main Menu Bar
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	// Console
	ImGui::Begin("Console", 0, ImGuiWindowFlags_NoFocusOnAppearing);
	{
		ImGui::Text(console_buffer.c_str());
	}

	ImGui::End();

	return UPDATE_CONTINUE;
}

// Load assets
bool ModuleEditor::CleanUp()
{
	ImGui_ImplSdlGL3_Shutdown();
	return true;
}

void ModuleEditor::Draw()
{
	ImGui::Render();
}

void ModuleEditor::HandleSDLEvent(SDL_Event* e)
{
	ImGui_ImplSdlGL3_ProcessEvent(e);
}

void ModuleEditor::AddTextConsole(const char* text)
{
	console_buffer.append(text);
}