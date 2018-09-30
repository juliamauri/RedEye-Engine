#include "ModuleEditor.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "RE_Math.h"
#include "OutputLog.h"
#include "MathGeoLib\include\MathGeoLib.h"

#include "ImGui\imgui_impl_sdl_gl3.h"
#include "ImGuizmo\ImGuizmo.h"
#include "glew\include\glew.h"
#include "SDL2\include\SDL.h"


ModuleEditor::ModuleEditor(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	windows.push_back(console = new ConsoleWindow());
	windows.push_back(config = new ConfigWindow());
	//windows.push_back(heriarchy = new HeriarchyWindow());
	//windows.push_back(properties = new PropertiesWindow());
	about = new AboutWindow();

	tools.push_back(rng = new RandomTest());
	tools.push_back(renderer = new RendererTest());
	tools.push_back(geo_test = new GeometryTest());
}

ModuleEditor::~ModuleEditor()
{
	windows.clear();
	tools.clear();
}

// Load assets
bool ModuleEditor::Init(JSONNode* node)
{
	bool ret = ImGui_ImplSdlGL3_Init(App->window->GetWindow());

	if (ret)
		App->ReportSoftware("ImGui", IMGUI_VERSION, "https://github.com/ocornut/imgui");

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

	if (about != nullptr)
		delete about;

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
	if (console != nullptr && !windows.empty())
	{
		if (console->filter < 0 || App->log->logHistory.back().first == console->filter)
		{
			console->console_buffer.append(App->log->logHistory.back().second.c_str());
			console->scroll_to_bot = true;
		}
	}
}

bool ModuleEditor::AddSoftwareUsed(SoftwareInfo s)
{
	bool ret = false;

	if (about)
	{
		about->sw_info.push_back(s);
		ret = true;
	}

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

const char * EditorWindow::Name() const
{
	return name;
}

bool EditorWindow::IsActive() const
{
	return active;
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
		if (ImGui::Button("All")) ChangeFilter(-1);

		std::map<std::string, unsigned int>::iterator it = App->log->callers.begin();
		for (; it != App->log->callers.end(); it++)
		{
			ImGui::SameLine();
			if (ImGui::Button(it->first.c_str())) ChangeFilter(it->second);
		}

		ImGui::TextUnformatted(console_buffer.begin());

		if (scroll_to_bot) ImGui::SetScrollHere(1.0f);
		scroll_to_bot = false;
	}

	ImGui::End();
}

void ConsoleWindow::ChangeFilter(int new_filter)
{
	if (new_filter != filter)
	{
		filter = new_filter;
		console_buffer.clear();
		scroll_to_bot = true;

		std::list<std::pair<unsigned int, std::string>>::iterator it = App->log->logHistory.begin();
		
		if (new_filter < 0)
		{
			for (; it != App->log->logHistory.end(); it++)
				console_buffer.append(it->second.c_str());
		}
		else
		{
			for (; it != App->log->logHistory.end(); it++)
			{
				if (it->first == filter)
					console_buffer.append(it->second.c_str());
			}
		}
	}
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

HeriarchyWindow::HeriarchyWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{}

void HeriarchyWindow::Draw()
{
	// draw gameobject tree
}

PropertiesWindow::PropertiesWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{}

void PropertiesWindow::Draw()
{
	// draw transform and components
}

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

RendererTest::RendererTest(const char * name, bool start_active) : EditorWindow(name,start_active)
{}


void RendererTest::Draw()
{
	ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing);
	{
		ModuleRenderer3D* renderer_module = App->renderer3d;

		bool* B_EBO = renderer_module->GetB_EBO();
		if (ImGui::Checkbox((*B_EBO) ? "Change to Triangle" : "Change to Square", B_EBO))
		{
			if (renderer_module->GetShaderEnabled() == ShaderType::TEXTURE)
			{
				renderer_module->SetShaderEnabled(ShaderType::SIN);
				shader_selcted = 0;
			}
		}

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
								renderer_module->ResetCamera();
					}
				}
			}
		}
	}

	ImGui::End();
}

GeometryTest::GeometryTest(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{}

void GeometryTest::Draw()
{
	ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing);
	{
		ImGui::Combo("Figure 1", &first_type, "Sphere\0Capsule\0AABB\0OBB\0Frustum\0Plane");
		ImGui::SliderFloat("F1-Pos X", &first_x, -10, 10);
		ImGui::SliderFloat("F1-Pos Y", &first_y, -10, 10);
		ImGui::SliderFloat("F1-Pos Z", &first_z, -10, 10);
		ImGui::SliderFloat("F1-Extra value", &first_xtra, -10, 10);

		ImGui::Separator();
		ImGui::Combo("Figure 2", &second_type, "Sphere\0Capsule\0AABB\0OBB\0Frustum\0Plane");
		ImGui::SliderFloat("F2-Pos X", &second_x, -10, 10);
		ImGui::SliderFloat("F2-Pos Y", &second_y, -10, 10);
		ImGui::SliderFloat("F2-Pos Z", &second_z, -10, 10);
		ImGui::SliderFloat("F2-Extra value", &second_xtra, -10, 10);

		ImGui::Separator();
		if (ImGui::Button("Check Intersection"))
		{
			math::Sphere sphere;
			math::Capsule capsule;
			math::AABB aabb;
			math::OBB obb;
			math::Frustum frustum, tmp;
			math::Plane plane;
			fig1 = "Figure 1: ";
			fig2 = "Figure 2: ";
			
			switch (GeoFigureType(first_type)) {
			case GEO_SPHERE:
				fig1 += "Sphere with center at Pos(F1-Pos X, F1-Pos Y, F1-Pos Z) and radius(F1-Extra value)";
				sphere = math::Sphere(math::vec(first_x, first_y, first_z), first_xtra);
				switch (GeoFigureType(second_type)) {
				case GEO_SPHERE:
					fig2 += "Sphere with center at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra));
					break;
				case GEO_CAPSULE:
					fig2 += "Capsule with bottom point(F2-Pos X, F2-Pos Y, F2-Pos Z), top point(F2-Pos X, (F2-Pos Y) + 2, F2-Pos Z) and radius(F2-Extra value))";
					intersects = sphere.Intersects(math::Capsule(math::vec(second_x, second_y, second_z), math::vec(second_x, second_y + 2.0f, second_z), second_xtra));
					break;
				case GEO_AABB:
					fig2 += "AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra)));
					break;
				case GEO_OBB:
					fig2 += "OBB made from AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::OBB(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra))));
					break;
				case GEO_FRUSTUM:
					fig2 += "Projective Frustum at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) with near plane at 0.1 and far plane at 10";
					tmp.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);
					tmp.SetPerspective(4.f, 3.f);
					tmp.SetWorldMatrix(math::float3x4::Translate(math::float3(0.0f, 0.0f, 0.0f)));
					tmp.SetPos(math::vec(second_x, second_y, second_z));
					tmp.SetViewPlaneDistances(0.1f, 10.f);
					intersects = sphere.Intersects(tmp);
					break;
				case GEO_PLANE:
					fig2 += "Plane at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) facing UP";
					intersects = sphere.Intersects(math::Plane(math::vec(first_x, first_y, first_z), math::vec(0.f, 1.f, 0.f)));
					break;
				}
				break;
			case GEO_CAPSULE:
				fig1 += "Capsule with bottom point(F1-Pos X, F1-Pos Y, F1-Pos Z), top point(F1-Pos X, (F1-Pos Y) + 2, F1-Pos Z) and radius(F1-Extra value))";
				capsule = math::Capsule(math::vec(first_x, first_y, first_z), math::vec(first_x, first_y + 2.0f, first_z), first_xtra);
				switch (GeoFigureType(second_type)) {
				case GEO_SPHERE:
					fig2 += "Sphere with center at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra));
					break;
				case GEO_CAPSULE:
					fig2 += "Capsule with bottom point(F2-Pos X, F2-Pos Y, F2-Pos Z), top point(F2-Pos X, (F2-Pos Y) + 2, F2-Pos Z) and radius(F2-Extra value))";
					intersects = sphere.Intersects(math::Capsule(math::vec(second_x, second_y, second_z), math::vec(second_x, second_y + 2.0f, second_z), second_xtra));
					break;
				case GEO_AABB:
					fig2 += "AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra)));
					break;
				case GEO_OBB:
					fig2 += "OBB made from AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::OBB(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra))));
					break;
				case GEO_FRUSTUM:
					fig2 += "Projective Frustum at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) with near plane at 0.1 and far plane at 10";
					tmp.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);
					tmp.SetPerspective(4.f, 3.f);
					tmp.SetWorldMatrix(math::float3x4::Translate(math::float3(0.0f, 0.0f, 0.0f)));
					tmp.SetPos(math::vec(second_x, second_y, second_z));
					tmp.SetViewPlaneDistances(0.1f, 10.f);
					intersects = sphere.Intersects(tmp);
					break;
				case GEO_PLANE:
					fig2 += "Plane at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) facing UP";
					intersects = sphere.Intersects(math::Plane(math::vec(first_x, first_y, first_z), math::vec(0.f, 1.f, 0.f)));
					break;
				}
				break;
			case GEO_AABB:
				fig1 += "AABB made from sphere at Pos(F1-Pos X, F1-Pos Y, F1-Pos Z) and radius(F1-Extra value)";
				aabb = math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra));
				switch (GeoFigureType(second_type)) {
				case GEO_SPHERE:
					fig2 += "Sphere with center at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra));
					break;
				case GEO_CAPSULE:
					fig2 += "Capsule with bottom point(F2-Pos X, F2-Pos Y, F2-Pos Z), top point(F2-Pos X, (F2-Pos Y) + 2, F2-Pos Z) and radius(F2-Extra value))";
					intersects = sphere.Intersects(math::Capsule(math::vec(second_x, second_y, second_z), math::vec(second_x, second_y + 2.0f, second_z), second_xtra));
					break;
				case GEO_AABB:
					fig2 += "AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra)));
					break;
				case GEO_OBB:
					fig2 += "OBB made from AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::OBB(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra))));
					break;
				case GEO_FRUSTUM:
					fig2 += "Projective Frustum at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) with near plane at 0.1 and far plane at 10";
					tmp.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);
					tmp.SetPerspective(4.f, 3.f);
					tmp.SetWorldMatrix(math::float3x4::Translate(math::float3(0.0f, 0.0f, 0.0f)));
					tmp.SetPos(math::vec(second_x, second_y, second_z));
					tmp.SetViewPlaneDistances(0.1f, 10.f);
					intersects = sphere.Intersects(tmp);
					break;
				case GEO_PLANE:
					fig2 += "Plane at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) facing UP";
					intersects = sphere.Intersects(math::Plane(math::vec(first_x, first_y, first_z), math::vec(0.f, 1.f, 0.f)));
					break;
				}
				break;
			case GEO_OBB:
				fig1 += "OBB made from AABB made from sphere at Pos(F1-Pos X, F1-Pos Y, F1-Pos Z) and radius(F1-Extra value)";
				obb = math::OBB(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra)));
				switch (GeoFigureType(second_type)) {
				case GEO_SPHERE:
					fig2 += "Sphere with center at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra));
					break;
				case GEO_CAPSULE:
					fig2 += "Capsule with bottom point(F2-Pos X, F2-Pos Y, F2-Pos Z), top point(F2-Pos X, (F2-Pos Y) + 2, F2-Pos Z) and radius(F2-Extra value))";
					intersects = sphere.Intersects(math::Capsule(math::vec(second_x, second_y, second_z), math::vec(second_x, second_y + 2.0f, second_z), second_xtra));
					break;
				case GEO_AABB:
					fig2 += "AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra)));
					break;
				case GEO_OBB:
					fig2 += "OBB made from AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::OBB(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra))));
					break;
				case GEO_FRUSTUM:
					fig2 += "Projective Frustum at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) with near plane at 0.1 and far plane at 10";
					tmp.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);
					tmp.SetPerspective(4.f, 3.f);
					tmp.SetWorldMatrix(math::float3x4::Translate(math::float3(0.0f, 0.0f, 0.0f)));
					tmp.SetPos(math::vec(second_x, second_y, second_z));
					tmp.SetViewPlaneDistances(0.1f, 10.f);
					intersects = sphere.Intersects(tmp);
					break;
				case GEO_PLANE:
					fig2 += "Plane at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) facing UP";
					intersects = sphere.Intersects(math::Plane(math::vec(first_x, first_y, first_z), math::vec(0.f, 1.f, 0.f)));
					break;
				}
				break;
			case GEO_FRUSTUM:
				fig1 += "Projective Frustum at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) with near plane at 0.1 and far plane at 10";
				frustum.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);
				frustum.SetPerspective(4.f, 3.f);
				frustum.SetWorldMatrix(math::float3x4::Translate(math::float3(0.0f, 0.0f, 0.0f)));
				frustum.SetPos(math::vec(second_x, second_y, second_z));
				frustum.SetViewPlaneDistances(0.1f, 10.f);
				switch (GeoFigureType(second_type)) {
				case GEO_SPHERE:
					fig2 += "Sphere with center at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra));
					break;
				case GEO_CAPSULE:
					fig2 += "Capsule with bottom point(F2-Pos X, F2-Pos Y, F2-Pos Z), top point(F2-Pos X, (F2-Pos Y) + 2, F2-Pos Z) and radius(F2-Extra value))";
					intersects = sphere.Intersects(math::Capsule(math::vec(second_x, second_y, second_z), math::vec(second_x, second_y + 2.0f, second_z), second_xtra));
					break;
				case GEO_AABB:
					fig2 += "AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra)));
					break;
				case GEO_OBB:
					fig2 += "OBB made from AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::OBB(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra))));
					break;
				case GEO_FRUSTUM:
					fig2 += "Projective Frustum at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) with near plane at 0.1 and far plane at 10";
					tmp.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);
					tmp.SetPerspective(4.f, 3.f);
					tmp.SetWorldMatrix(math::float3x4::Translate(math::float3(0.0f, 0.0f, 0.0f)));
					tmp.SetPos(math::vec(second_x, second_y, second_z));
					tmp.SetViewPlaneDistances(0.1f, 10.f);
					intersects = sphere.Intersects(tmp);
					break;
				case GEO_PLANE:
					fig2 += "Plane at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) facing UP";
					intersects = sphere.Intersects(math::Plane(math::vec(first_x, first_y, first_z), math::vec(0.f, 1.f, 0.f)));
					break;
				}
				break;
			case GEO_PLANE:
				fig1 += "Plane at Pos(F1-Pos X, F1-Pos Y, F1-Pos Z) facing UP";
				plane = math::Plane(math::vec(first_x, first_y, first_z), math::vec(0.f, 1.f, 0.f));
				switch (GeoFigureType(second_type)) {
				case GEO_SPHERE:
					fig2 += "Sphere with center at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra));
					break;
				case GEO_CAPSULE:
					fig2 += "Capsule with bottom point(F2-Pos X, F2-Pos Y, F2-Pos Z), top point(F2-Pos X, (F2-Pos Y) + 2, F2-Pos Z) and radius(F2-Extra value))";
					intersects = sphere.Intersects(math::Capsule(math::vec(second_x, second_y, second_z), math::vec(second_x, second_y + 2.0f, second_z), second_xtra));
					break;
				case GEO_AABB:
					fig2 += "AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra)));
					break;
				case GEO_OBB:
					fig2 += "OBB made from AABB made from sphere at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) and radius(F2-Extra value)";
					intersects = sphere.Intersects(math::OBB(math::AABB(math::Sphere(math::vec(second_x, second_y, second_z), second_xtra))));
					break;
				case GEO_FRUSTUM:
					fig2 += "Projective Frustum at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) with near plane at 0.1 and far plane at 10";
					tmp.SetKind(math::FrustumProjectiveSpace::FrustumSpaceGL, math::FrustumHandedness::FrustumRightHanded);
					tmp.SetPerspective(4.f, 3.f);
					tmp.SetWorldMatrix(math::float3x4::Translate(math::float3(0.0f, 0.0f, 0.0f)));
					tmp.SetPos(math::vec(second_x, second_y, second_z));
					tmp.SetViewPlaneDistances(0.1f, 10.f);
					intersects = sphere.Intersects(tmp);
					break;
				case GEO_PLANE:
					fig2 += "Plane at Pos(F2-Pos X, F2-Pos Y, F2-Pos Z) facing UP";
					intersects = sphere.Intersects(math::Plane(math::vec(first_x, first_y, first_z), math::vec(0.f, 1.f, 0.f)));
					break;
				}
				break;
			}

		}

		ImGui::TextWrappedV(fig1.c_str(), "");
		ImGui::Text("&");
		ImGui::TextWrappedV(fig2.c_str(), "");

		if (intersects)
			ImGui::Text("Intersect");
		else
			ImGui::Text("Do NOT Intersect");

	}

	ImGui::End();
}

SoftwareInfo::SoftwareInfo(const char * name, const char * v, const char * w) :
	name(name)
{
	if (v != nullptr) version = v;
	if (w != nullptr) website = w;
}
