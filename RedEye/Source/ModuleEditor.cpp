#include "ModuleEditor.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "RE_Math.h"

#include "ImGui\imgui_impl_sdl_gl3.h"
#include "ImGuizmo\ImGuizmo.h"
#include "glew\include\glew.h"
#include "SDL2\include\SDL.h"


ModuleEditor::ModuleEditor(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	windows.push_back(console = new ConsoleWindow());
	windows.push_back(config = new ConfigWindow());
	windows.push_back(rng = new RandomTest());
	windows.push_back(renderer = new RendererTest());
	show_demo = false;
}

ModuleEditor::~ModuleEditor()
{
	windows.clear();
}

// Load assets
bool ModuleEditor::Init(JSONNode* node)
{
	bool ret = ImGui_ImplSdlGL3_Init(App->window->GetWindow());

	// TODO set window lock positions

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
			std::list<EditorWindow*>::iterator it = windows.begin();
			for (; it != windows.end(); it++)
			{
				if (ImGui::MenuItem((*it)->Name(), ((**it)) ? "Hide" : "Open"))
				{
					(*it)->SwitchActive();
				}
			}
			ImGui::EndMenu();
		}

		// Help
		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem(show_demo ? "Close Gui Demo" : "Open Gui Demo"))
				show_demo = !show_demo;
			if (ImGui::MenuItem("Documentation"))
				App->RequestBrowser("https://github.com/juliamauri/RedEye-Engine/wiki");
			if (ImGui::MenuItem("Download Latest"))
				App->RequestBrowser("https://github.com/juliamauri/RedEye-Engine/releases");
			if (ImGui::MenuItem("Report a Bug"))
				App->RequestBrowser("https://github.com/juliamauri/RedEye-Engine/issues");

			ImGui::EndMenu();
		}

		if(show_demo) ImGui::ShowTestWindow();

		ImGui::EndMainMenuBar();
	}

	// Windows
	std::list<EditorWindow*>::iterator it = windows.begin();
	for (it; it != windows.end(); it++)
	{
		if ((**it)) (*it)->DrawWindow();
	}

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
	if(console != nullptr && !windows.empty())
		console->console_buffer.append(text);
}

///////////////////////////////////////////////////////////////////////
///////   Editor Windows   ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

EditorWindow::EditorWindow(const char* name, bool start_enabled)
	: name(name), active(start_enabled), lock_pos(false)
{}

void EditorWindow::DrawWindow()
{
	if (lock_pos)
	{
		ImGui::SetNextWindowPos(pos);
		ImGui::SetWindowSize(size);
	}

	Draw();
}

void EditorWindow::SwitchActive()
{
	active = !active;
}

const char * EditorWindow::Name() const
{
	return name;
}

EditorWindow::operator bool() const
{
	return active;
}

inline bool EditorWindow::operator!() const
{
	return !active;
}

ConsoleWindow::ConsoleWindow(const char * name, bool start_active) : 
	EditorWindow(name, start_active)
{
	pos.y = 500.f;
}

void ConsoleWindow::Draw()
{
	ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing);
	{
		ImGui::Text(console_buffer.c_str());
	}

	ImGui::End();
}

ConfigWindow::ConfigWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{
	changed_config = false;
	pos.x = 2000.f;
	pos.y = 400.f;
}

void ConfigWindow::Draw()
{
	ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing);
	{
		App->DrawEditor();
	}

	ImGui::End();
}

PropertiesWindow::PropertiesWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{
}

void PropertiesWindow::Draw()
{
}

HeriarchyWindow::HeriarchyWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{
}

void HeriarchyWindow::Draw()
{
}

RandomTest::RandomTest(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{}

void RandomTest::Draw()
{
	ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing);
	{
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
	}

	ImGui::End();
}

RendererTest::RendererTest(const char * name, bool start_active) : EditorWindow(name,start_active) {}

void RendererTest::Draw()
{
	ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing);
	{
		ModuleRenderer3D* renderer_module = App->renderer3d;

		bool* vsync = renderer_module->GetVsync();
		if (ImGui::Checkbox((*vsync) ? "Disable vsync" : "Enable vsync", vsync))
		{
			renderer_module->enableVSync(*vsync);
		}

		bool* B_EBO = renderer_module->GetB_EBO();
		if (ImGui::Checkbox((*B_EBO) ? "Change to Triangle" : "Change to Square", B_EBO))
		{
			if (renderer_module->GetShaderEnabled() == ShaderType::TEXTURE)
			{
				renderer_module->SetShaderEnabled(ShaderType::SIN);
				shader_selcted = 0;
			}
		}

		bool* isLine = renderer_module->GetisLine();
		ImGui::Checkbox((*isLine) ? "Disable Wireframe" : "Enable Wireframe", isLine);

		if (*B_EBO)
		{
			if (ImGui::Combo("Shader", &shader_selcted, "Sin\0Vertex\0Texture\0"))
			{
				switch ((ShaderType)shader_selcted)
				{
				case SIN:
					renderer_module->SetShaderEnabled(ShaderType::SIN);
					break;
				case VERTEX:
					renderer_module->SetShaderEnabled(ShaderType::VERTEX);
					break;
				case TEXTURE:
					renderer_module->SetShaderEnabled(ShaderType::TEXTURE);
					break;
				}
			}
		}
		else
		{
			if (ImGui::Combo("Shader", &shader_selcted, "Sin\0Vertex\0"))
			{
				switch ((ShaderType)shader_selcted)
				{
				case SIN:
					renderer_module->SetShaderEnabled(ShaderType::SIN);
					break;
				case VERTEX:
					renderer_module->SetShaderEnabled(ShaderType::VERTEX);
					break;
				}
			}
		}

		if (renderer_module->GetShaderEnabled() == ShaderType::TEXTURE)
		{
			if (ImGui::Combo("Texture", &texture_selected, "Puppie 1\0Puppie 2\0Container\0Mix Awesomeface\0"))
			{
				switch ((Texture2DType)texture_selected)
				{
				case PUPPIE_1:
					renderer_module->SetTexture2DEnabled(Texture2DType::PUPPIE_1);
					break;
				case PUPPIE_2:
					renderer_module->SetTexture2DEnabled(Texture2DType::PUPPIE_2);
					break;
				case CONTAINER:
					renderer_module->SetTexture2DEnabled(Texture2DType::CONTAINER);
					break;
				case MIX_AWESOMEFACE:
					renderer_module->SetTexture2DEnabled(Texture2DType::MIX_AWESOMEFACE);
					break;
				}
			}

			if (renderer_module->GetTexture2DEnabled() != Texture2DType::MIX_AWESOMEFACE)
			{
				bool* printvertextcolor = renderer_module->Getprintvertextcolor();
				if (ImGui::Button((!(*printvertextcolor)) ? "Activate Vertex Color" : "Deactivate Vertex Color"))
				{
					renderer_module->UseShader(ShaderType::TEXTURE);
					if (!(*printvertextcolor))
					{
						renderer_module->SetShaderBool("vertexcolor", true);
						*printvertextcolor = !(*printvertextcolor);
					}
					else
					{
						renderer_module->SetShaderBool("vertexcolor", false);
						*printvertextcolor = !(*printvertextcolor);
					}
				}
			}
			else if (renderer_module->GetTexture2DEnabled() == Texture2DType::MIX_AWESOMEFACE)
			{
				if (ImGui::Combo("Object Type", &object_selected, "Plane\0Cube"))
				{
					switch ((ObjectType)object_selected)
					{
					case PLANE:
						renderer_module->SetObjectEnabled(ObjectType::PLANE);
						break;
					case CUBE:
						renderer_module->SetObjectEnabled(ObjectType::CUBE);
						break;
					}
				}
				if (renderer_module->GetObjectEnabled() == ObjectType::PLANE)
				{
					bool* isRotated = renderer_module->GetisRotated();
					if (ImGui::Checkbox((*isRotated) ? "ToPlane3D" : "ToPlane2D", isRotated))
					{
						renderer_module->UseShader(ShaderType::TEXTURE);
						if (*isRotated)
							renderer_module->SetShaderBool("rotation", true);
						else
							renderer_module->SetShaderBool("rotation", false);
					}

					if (*isRotated)
					{
						bool* isScaled = renderer_module->GetisScaled();
						ImGui::Checkbox((*isScaled) ? "ToScale" : "ToNormal", isScaled);
					}
				}
				else
				{
					bool* isCubes = renderer_module->GetisCubes();
					ImGui::Checkbox((*isCubes) ? "ToCube" : "ToCubes", isCubes);

					if (*isCubes)
					{
						bool* isMove = renderer_module->GetisMove();
						if(ImGui::Checkbox((*isMove) ? "ToLookAround" : "ToMove", isMove))
							if (*isMove)
								renderer_module->ResetCameraPos();
					}
				}
			}
		}
	}

	ImGui::End();
}
