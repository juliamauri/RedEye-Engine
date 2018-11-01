#include "ModuleEditor.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "ModuleScene.h"
#include "Texture2DManager.h"
#include "RE_Math.h"
#include "OutputLog.h"
#include "MathGeoLib\include\MathGeoLib.h"

#include "ImGui\imgui_impl_sdl_gl3.h"
#include "ImGuizmo\ImGuizmo.h"
#include "glew\include\glew.h"
#include "SDL2\include\SDL.h"

SoftwareInfo::SoftwareInfo(const char * name, const char * v, const char * w) :
	name(name)
{
	if (v != nullptr) version = v;
	if (w != nullptr) website = w;
}

ModuleEditor::ModuleEditor(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	windows.push_back(console = new ConsoleWindow());
	windows.push_back(config = new ConfigWindow());
	//windows.push_back(heriarchy = new HeriarchyWindow());
	windows.push_back(properties = new PropertiesWindow());
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
				std::list<EditorWindow*>::iterator it = windows.begin();
				for (; it != windows.end(); it++)
				{
					if (ImGui::MenuItem((*it)->Name(), ((**it)) ? "Hide" : "Open"))
					{
						(*it)->SwitchActive();
					}
				}

				// tools submenu
				if (ImGui::BeginMenu("Tools"))
				{
					it = tools.begin();
					for (; it != tools.end(); it++)
					{
						if (ImGui::MenuItem((*it)->Name(), ((**it)) ? "Hide" : "Open"))
						{
							(*it)->SwitchActive();
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
		std::list<EditorWindow*>::iterator it = windows.begin();
		for (it; it != windows.end(); it++)
		{
			if ((**it)) (*it)->DrawWindow();
		}

		if (about && about->IsActive()) about->DrawWindow(); // (not in windows' list)

		it = tools.begin();
		for (it; it != tools.end(); it++)
		{
			if ((**it)) (*it)->DrawWindow();
		}

	}
	
	if(App->input->CheckKey(SDL_SCANCODE_F1))
		show_all = !show_all;

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

void ModuleEditor::Draw()
{
	ImGui::Render();
}

void ModuleEditor::HandleSDLEvent(SDL_Event* e)
{
	ImGui_ImplSdlGL3_ProcessEvent(e);
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

bool ModuleEditor::AddSoftwareUsed(SoftwareInfo s)
{
	bool ret = (about != nullptr);

	if (ret) about->sw_info.push_back(s);

	return ret;
}

///////////////////////////////////////////////////////////////////////
///////   Editor Windows   ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

EditorWindow::EditorWindow(const char* name, bool start_enabled)
	: name(name), active(start_enabled), lock_pos(false)
{}

EditorWindow::~EditorWindow()
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

const char * EditorWindow::Name() const { return name; }

bool EditorWindow::IsActive() const { return active; }

EditorWindow::operator bool() const { return active; }

inline bool EditorWindow::operator!() const { return !active; }


///////   Console Window   ////////////////////////////////////////////

ConsoleWindow::ConsoleWindow(const char * name, bool start_active) : 
	EditorWindow(name, start_active)
{
	pos.y = 500.f;
}

void ConsoleWindow::Draw()
{
	ImGui::Begin(name, 0, ImGuiWindowFlags_MenuBar);// | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoFocusOnAppearing);
	{
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

		ImGui::TextWrapped(console_buffer.begin());
		if (scroll_to_bot)
		{
			ImGui::SetScrollHere(1.f);
			scroll_to_bot = false;
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

void ConfigWindow::Draw()
{
	ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_HorizontalScrollbar);
	{
		App->DrawEditor();
	}

	ImGui::End();
}


///////   Heriarchy Window   ////////////////////////////////////////////

HeriarchyWindow::HeriarchyWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{}

void HeriarchyWindow::Draw()
{
	// draw gameobject tree
}


///////   Properties Window   ////////////////////////////////////////////

PropertiesWindow::PropertiesWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{}

void PropertiesWindow::Draw()
{
	// draw transform and components
	ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_HorizontalScrollbar);
	{
		if (App->scene) App->scene->DrawFocusedProperties();
	}

	ImGui::End();
}


///////   About Window   ////////////////////////////////////////////

AboutWindow::AboutWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{}

void AboutWindow::Draw()
{
	ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing);
	{
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
	}

	ImGui::End();
}


///////   Random Tool   ////////////////////////////////////////////

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


///////   Texture Manager  ////////////////////////////////////////////
TexturesWindow::TexturesWindow(const char* name, bool start_active)  : EditorWindow(name, start_active) {}

void TexturesWindow::Draw()
{
	ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing);
	{
		std::vector<Texture2D*>* textures = App->textures->GetTextures();
		if (textures->size() != 0)
		{
			std::vector<Texture2D*>::iterator it = textures->begin();
			for (it; it != textures->end(); ++it) {

				if (ImGui::TreeNode((*it)->GetName()))
				{
					int widht, height;
					(*it)->GetWithHeight(&widht, &height);
					ImGui::Text("GL ID: %u", (*it)->GetID());
					ImGui::Text("Widht: %u", widht);
					ImGui::Text("Height: %u", height);
					(*it)->DrawTextureImGui();
					ImGui::TreePop();
				}
			}
		}
	}
	ImGui::End();
}
