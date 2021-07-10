#include "EditorWindows.h"

#include "Application.h"
#include "RE_Time.h"
#include "RE_Math.h"
#include "RE_Hardware.h"
#include "RE_FileSystem.h"

#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "ModulePhysics.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3d.h"
#include "ModuleAudio.h"

#include "RE_ResourceManager.h"
#include "RE_ParticleEmitterBase.h"
#include "RE_CameraManager.h"
#include "RE_ThumbnailManager.h"
#include "RE_GameObject.h"
#include "RE_Prefab.h"
#include "RE_Command.h"

#include "ImGui/misc/cpp/imgui_stdlib.h"
#include "ImGui/imgui_internal.h"
#include "ImGuiWidgets/ImGuiColorTextEdit/TextEditor.h"
#include "SDL2/include/SDL_scancode.h"
#include <EAStdC/EASprintf.h>

EditorWindow::EditorWindow(const char* name, bool start_enabled) : name(name), active(start_enabled), lock_pos(false) {}
EditorWindow::~EditorWindow() {}

void EditorWindow::DrawWindow(bool secondary)
{
	if (lock_pos)
	{
		ImGui::SetNextWindowPos(pos);
		ImGui::SetWindowSize(size);
	}

	Draw(secondary);
}

void EditorWindow::SwitchActive() { active = !active; }
const char * EditorWindow::Name() const { return name; }
bool EditorWindow::IsActive() const { return active; }


///////   Console Window   ////////////////////////////////////////////
ConsoleWindow::ConsoleWindow(const char * name, bool start_active) :
	EditorWindow(name, start_active)
{
	pos.y = 500.f;
	for (auto &c : categories) c = true;
}

ConsoleWindow::~ConsoleWindow() {}

void ConsoleWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_MenuBar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Filter Files"))
			{
				if (ImGui::MenuItem("All")) ChangeFilter(-1);

				eastl::map<eastl::string, unsigned int>::iterator it = callers.begin();
				for (int i = 0; it != callers.end(); i++, it++)
					if (ImGui::MenuItem(it->first.c_str()))
						ChangeFilter(it->second);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Filter Categories"))
			{
				static const char* category_names[8] = { "Separator", "Global", "Secondary", "Terciary", "Software", "Error" , "Warning", "Solution" };
				for (unsigned int j = 0; j < L_TOTAL_CATEGORIES; j++)
					if (ImGui::MenuItem(category_names[j], categories[j] ? "Hide" : "Show"))
						SwapCategory(j);

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		//Draw console buffer
		if (needs_rewriting) ResetBuffer();
		ImGui::TextEx(console_buffer.begin(), console_buffer.end(), ImGuiTextFlags_NoWidthForLargeClippedText);

		if (scroll_to_bot)
		{
			ImGui::SetScrollHere(1.f);
			scroll_to_bot = false;
		}
		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

void ConsoleWindow::ResetBuffer()
{
	needs_rewriting = false;
	console_buffer.clear();

	for (auto log : logHistory)
	{
		if (categories[log.category] && (file_filter < 0 || log.caller_id == file_filter))
		{
			console_buffer.append(log.data.c_str());

			if (log.count > 1u)
				console_buffer.append((" -> Called: " + eastl::to_string(log.count) + " times\n").c_str());
		}
	}
}

void ConsoleWindow::ChangeFilter(const int new_filter)
{
	if (new_filter != file_filter)
	{
		file_filter = new_filter;
		scroll_to_bot = needs_rewriting = true;
	}
}

void ConsoleWindow::SwapCategory(const unsigned int c)
{
	categories[c] = !categories[c];
	scroll_to_bot = needs_rewriting = true;
}

void ConsoleWindow::AppendLog(unsigned int category, const char* text, const char* file_name)
{
	static unsigned int next_caller_id = 0u;
	unsigned int count = 1u;
	auto caller_id = callers.find(file_name);
	if (caller_id != callers.end())
	{
		for (auto it = logHistory.rbegin(); it != logHistory.rend(); ++it)
		{
			if (it->caller_id == caller_id->second
				&& it->category == category
				&& (eastl::Compare(it->data.c_str(), text, it->data.size()) == 0))
			{
				if (it != logHistory.rbegin())
				{
					count += it->count;
					logHistory.erase(it);
					logHistory.push_back({ caller_id->second, category, count, text });
				}
				else logHistory.rbegin()->count++;

				scroll_to_bot = needs_rewriting = true;
				return;
			}
		}

		logHistory.push_back({ caller_id->second, category, count, text });
	}
	else 
	{
		callers.insert(eastl::pair<eastl::string, unsigned int>(file_name, ++next_caller_id));
		logHistory.push_back({ next_caller_id, category, count, text });
	}

	if (!needs_rewriting && categories[category] && (file_filter < 0 || logHistory.back().caller_id == file_filter))
	{
		console_buffer.append(text);
		scroll_to_bot = true;
	}
}


///////   Configuration Window   ////////////////////////////////////////////
ConfigWindow::ConfigWindow(const char * name, bool start_active) : EditorWindow(name, start_active)
{
	changed_config = false;
	pos.x = 2000.f;
	pos.y = 400.f;
}

ConfigWindow::~ConfigWindow() {}

void ConfigWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::BeginMenu("Options"))
		{
			if (ImGui::MenuItem("Load")) RE_INPUT->Push(REQUEST_LOAD, App);
			if (ImGui::MenuItem("Save")) RE_INPUT->Push(REQUEST_SAVE, App);
			ImGui::EndMenu();
		}

		if (ImGui::CollapsingHeader("Time Profiling")) RE_TIME->DrawEditorGraphs();
		
		RE_INPUT->DrawEditor();
		RE_WINDOW->DrawEditor();
		RE_SCENE->DrawEditor();
		RE_PHYSICS->DrawEditor();
		RE_EDITOR->DrawEditor();
		RE_RENDER->DrawEditor();
		RE_AUDIO->DrawEditor();

		if (ImGui::CollapsingHeader("File System")) RE_FS->DrawEditor();
		if (ImGui::CollapsingHeader("Hardware")) RE_HARDWARE->DrawEditor();

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}


///////   Heriarchy Window   ////////////////////////////////////////////
HeriarchyWindow::HeriarchyWindow(const char * name, bool start_active) : EditorWindow(name, start_active) {}
HeriarchyWindow::~HeriarchyWindow() {}

void HeriarchyWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if(RE_SCENE) RE_EDITOR->DrawHeriarchy();

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}


///////   Properties Window   ////////////////////////////////////////////
PropertiesWindow::PropertiesWindow(const char * name, bool start_active) : EditorWindow(name, start_active) {}
PropertiesWindow::~PropertiesWindow() {}

void PropertiesWindow::Draw(bool secondary)
{
	// draw transform and components
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (RE_RES->GetSelected() != nullptr) RE_RES->At(RE_RES->GetSelected())->DrawPropieties();
		else if (RE_EDITOR->GetSelected()) RE_SCENE->GetGOPtr(RE_EDITOR->GetSelected())->DrawProperties();

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

///////   About Window   ////////////////////////////////////////////
AboutWindow::AboutWindow(const char * name, bool start_active) : EditorWindow(name, start_active) {}
AboutWindow::~AboutWindow() {}

void AboutWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Engine name: %s", ENGINE_NAME);
		ImGui::Text("Version: %s", ENGINE_VERSION);
		ImGui::Text("Organization: %s", ENGINE_ORGANIZATION);
		ImGui::Text("License: %s", ENGINE_LICENSE);

		ImGui::Separator();
		ImGui::Text(ENGINE_DESCRIPTION);

		ImGui::Separator();
		ImGui::Text("Authors:");
		ImGui::Text(ENGINE_AUTHOR_1);
		ImGui::SameLine();
		if (ImGui::Button("Visit Github Profile")) BROWSER("https://github.com/juliamauri");
		ImGui::Text(ENGINE_AUTHOR_2);
		ImGui::SameLine();
		if (ImGui::Button("Visit Github Profile")) BROWSER("https://github.com/cumus");

		ImGui::Separator();
		if (ImGui::CollapsingHeader("3rd Party Software:"))
		{
			for (auto software : sw_info)
			{
				if (software.version != nullptr) ImGui::BulletText("%s: v%s ", software.name, software.version);
				else ImGui::BulletText("%s ", software.name);

				if (software.website != nullptr)
				{
					ImGui::SameLine();
					char tmp[128];
					EA::StdC::Snprintf(tmp, 128, "Open %s Website", software.name);
					if (ImGui::Button(tmp)) BROWSER(software.website);
				}
			}
		}

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}


///////   Random Tool   ////////////////////////////////////////////
RandomTest::RandomTest(const char * name, bool start_active) : EditorWindow(name, start_active) {}
RandomTest::~RandomTest() {}

void RandomTest::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::Text("Random Integer");
		ImGui::SliderInt("Min Integer", &minInt, 0, maxInt);
		ImGui::SliderInt("Max Integer", &maxInt, minInt, 100);

		if (ImGui::Button("Generate Int"))
			resultInt = RE_MATH->RandomInt(minInt, maxInt);

		ImGui::SameLine();
		ImGui::Text("Random Integer: %u", resultInt);

		ImGui::Separator();

		ImGui::Text("Random Float");
		ImGui::SliderFloat("Min Float", &minF, -100.f, maxF, "%.1f");
		ImGui::SliderFloat("Max Float", &maxF, minF, 100.f, "%.1f");

		if (ImGui::Button("Generate Float"))
			resultF = RE_MATH->RandomF(minF, maxF);

		ImGui::SameLine();
		ImGui::Text("Random Float: %.2f", resultF);

		ImGui::Separator();

		ImGui::Text("Random UID");

		ImGui::SliderInt("Loops 1 per frame", &loops[0], 1, 65535);
		ImGui::SliderInt("Loops 2 per frame", &loops[1], 1, 65535);

		ImGui::Text("First: %s", eastl::to_string(first).c_str());
		ImGui::Text("Max: %s", eastl::to_string(max).c_str());
		ImGui::Text("Min: %s", eastl::to_string(min).c_str());
		ImGui::Text("Count: %s", eastl::to_string(count).c_str());
		if (generating)
		{
			RE_Timer timer;

			for (int i = 0; i < loops[0]; ++i)
			{
				for (int i = 0; i < loops[1]; ++i)
				{
					UID r = RE_MATH->RandomUID();
					if (r == first)
					{
						generating = false;
					}
					else
					{
						count++;
						if (r < min) min = r;
						if (r > max) max = r;
					}
				}
			}

			ImGui::Text("Time per %d loops: %u", loops, timer.Read());
		}
		else if (ImGui::Button("Generate UID"))
		{
			generating = true;
			first = RE_MATH->RandomUID();
			max = count = 0;
			min = 0xffffffffffffffff;
		}

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

///////   Play Pause   ////////////////////////////////////////////
PlayPauseWindow::PlayPauseWindow(const char * name, bool start_active) : EditorWindow(name, start_active) {}
PlayPauseWindow::~PlayPauseWindow() {}

void PlayPauseWindow::Draw(bool secondary)
{
	if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (RE_CameraManager::HasMainCamera())
		{
			switch (RE_TIME->DrawEditorControls()) {
			case GS_PLAY:  RE_INPUT->Push(PLAY,  App); break;
			case GS_PAUSE: RE_INPUT->Push(PAUSE, App); break;
			case GS_STOP:  RE_INPUT->Push(STOP,  App); break;
			case GS_TICK:  RE_INPUT->Push(TICK,  App); break;
			default: break; }
		}
		else ImGui::Text("Missing Main Camera");

		ImGui::SameLine();
		ImGui::Checkbox("Draw Gizmos", &RE_EDITOR->debug_drawing);

		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_::ImGuiSeparatorFlags_Vertical);

		static ImGuizmo::OPERATION o = RE_EDITOR->GetSceneEditor()->GetOperation();
		static bool changed = false;
		static bool colored = false;
		ImGui::SameLine();

		if (RE_INPUT->GetKey(SDL_SCANCODE_Q) == KEY_STATE::KEY_DOWN){ o = ImGuizmo::OPERATION::TRANSLATE; changed = true; }
		if (RE_INPUT->GetKey(SDL_SCANCODE_W) == KEY_STATE::KEY_DOWN){ o = ImGuizmo::OPERATION::ROTATE;    changed = true; }
		if (RE_INPUT->GetKey(SDL_SCANCODE_E) == KEY_STATE::KEY_DOWN){ o = ImGuizmo::OPERATION::SCALE;	    changed = true; }

		if (!colored && o == ImGuizmo::OPERATION::TRANSLATE) {
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 1.0f });
			colored = true;
		}

		if (ImGui::Button("Translate")) {
			o = ImGuizmo::OPERATION::TRANSLATE;
			changed = true;
		}

		if (colored) {
			ImGui::PopStyleColor();
			colored = false;
		}

		if (!colored && !changed && o == ImGuizmo::OPERATION::ROTATE) {
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 1.0f });
			colored = true;
		}

		ImGui::SameLine();
		if (ImGui::Button("Rotate")) {
			o = ImGuizmo::OPERATION::ROTATE;
			changed = true;
		}

		if (colored) {
			ImGui::PopStyleColor();
			colored = false;
		}

		if (!colored && !changed && o == ImGuizmo::OPERATION::SCALE) {
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 1.0f });
			colored = true;
		}

		ImGui::SameLine();
		if (ImGui::Button("Scale")) {
			o = ImGuizmo::OPERATION::SCALE;
			changed = true;
		}

		if (colored) {
			ImGui::PopStyleColor();
			colored = false;
		}

		if (changed) {
			RE_EDITOR->GetSceneEditor()->SetOperation(o);
			changed = false;
		}


		ImGui::SameLine();
		static ImGuizmo::MODE m = RE_EDITOR->GetSceneEditor()->GetMode();
		if (ImGui::Button((m == ImGuizmo::MODE::LOCAL) ? "Local Transformation" : "Global Transformation"))
		{
			switch (m) {
			case ImGuizmo::MODE::LOCAL: m = ImGuizmo::MODE::WORLD; break;
			case ImGuizmo::MODE::WORLD: m = ImGuizmo::MODE::LOCAL; break; }
			RE_EDITOR->GetSceneEditor()->SetMode(m);
		}

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
	ImGui::End();
}

///////   PopUp Window   ////////////////////////////////////////////
PopUpWindow::PopUpWindow(const char * name, bool start_active) : EditorWindow(name, start_active) {}
PopUpWindow::~PopUpWindow() {}

void PopUpWindow::PopUp(const char * _btnText, const char* title, bool _disableAllWindows)
{
	btn_text = _btnText;
	title_text = title;
	RE_EDITOR->PopUpFocus(_disableAllWindows);
	active = true;
}

void PopUpWindow::PopUpError()
{
	state = PU_ERROR;
	PopUp("Accept", "Error", true);
}

void PopUpWindow::PopUpSaveScene(bool fromExit, bool newScene)
{
	state = PU_SAVE_SCENE;
	exit_after = fromExit;
	spawn_new_scene = newScene;
	if (input_name = RE_SCENE->isNewScene()) name_str = "New Scene";
	PopUp("Save", "Scene have changes", true);
}

void PopUpWindow::PopUpSaveParticles(bool need_particle_names, bool not_name, bool not_emissor, bool not_renderer, bool close_after)
{
	state = PU_SAVE_PARTICLEEMITTER;
	exit_after = close_after;
	particle_names = need_particle_names;
	if (particle_names) {
		name_str = "emitter_name";
		need_name = !not_name;
		emission_name = "emission_name";
		need_emission = !not_emissor;
		renderer_name = "renderer_name";
		need_renderer = !not_renderer;
	}
	PopUp("Save", "Save particle emittter", true);
}

void PopUpWindow::PopUpPrefab(RE_GameObject* go)
{
	state = PU_PREFAB;
	input_name = true;
	name_str = "New Prefab";
	go_prefab = go;
	PopUp("Save", "Create prefab", false);
}

void PopUpWindow::PopUpDelRes(const char* res)
{
	//TODO PARTICLE RESOUCES
	state = PU_DELETERESOURCE;
	resource_to_delete = res;
	using_resources = RE_RES->WhereIsUsed(res);

	Resource_Type rType = RE_RES->At(res)->GetType();
	
	eastl::stack<RE_Component*> comps;

	switch (rType)
	{
	case R_SKYBOX:
	{
		comps = RE_SCENE->GetScenePool()->GetRootPtr()->GetAllChildsComponents(C_CAMERA);
		break;
	}
	case R_MATERIAL:
	{
		comps = RE_SCENE->GetScenePool()->GetRootPtr()->GetAllChildsComponents(C_MESH);
		break;
	}
	case R_PARTICLE_EMISSION:
	case R_PARTICLE_RENDER:
	case R_PARTICLE_EMITTER:
		comps = RE_SCENE->GetScenePool()->GetRootPtr()->GetAllChildsComponents(C_PARTICLEEMITER);
		break;
	}
	
	bool skip = false;
	while (!comps.empty() && !skip)
	{
		switch (rType)
		{
		case R_SKYBOX:
		{
			RE_CompCamera* cam = dynamic_cast<RE_CompCamera*>(comps.top());
			if (cam && cam->GetSkybox() == res)
			{
				resource_on_scene = true;
				skip = true;
			}
			break;
		}
		case R_MATERIAL:
		{
			RE_CompMesh* mesh = dynamic_cast<RE_CompMesh*>(comps.top());
			if (mesh && mesh->GetMaterial() == res)
			{
				resource_on_scene = true;
				skip = true;
			}
			break;
		}
		case R_PARTICLE_EMITTER:
		{
			RE_CompParticleEmitter* emitter = dynamic_cast<RE_CompParticleEmitter*>(comps.top());
			if (emitter && emitter->GetEmitterResource() == res)
			{
				resource_on_scene = true;
				skip = true;
			}
			break;
		}
		case R_PARTICLE_EMISSION:
		case R_PARTICLE_RENDER:
		{
			RE_CompParticleEmitter* emitter = dynamic_cast<RE_CompParticleEmitter*>(comps.top());

			if (emitter)
			{
				const char* emitter_res = emitter->GetEmitterResource();
				if (emitter_res)
				{
					if (dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emitter_res))->Contains(res))
					{
						resource_on_scene = true;
						skip = true;
					}
				}
			}
			break;
		}
		}

		comps.pop();
	}

	PopUp("Delete", "Do you want to delete that resource?", false);
}

void PopUpWindow::PopUpDelUndeFile(const char* assetPath)
{
	state = PU_DELETEUNDEFINEDFILE;
	name_str = assetPath;
	using_resources = RE_RES->WhereUndefinedFileIsUsed(assetPath);
	PopUp("Delete", "Do you want to delete that file?", false);
}

void PopUpWindow::AppendScopedLog(const char* log, unsigned int type)
{
	if (log != nullptr)
	{
		logs += log;
		switch (type) {
		case CONSOLE_LOG_SAVE_ERROR: errors += log; break;
		case CONSOLE_LOG_SAVE_WARNING: warnings += log; break;
		case CONSOLE_LOG_SAVE_SOLUTION: solutions += log; break;
		default: break; }
	}
}

void PopUpWindow::ClearScope()
{
	logs.clear();
	errors.clear();
	warnings.clear();
	solutions.clear();
}

void PopUpWindow::Draw(bool secondary)
{
	if(ImGui::Begin(title_text.c_str(), 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		switch (state)
		{
		case PopUpWindow::PU_NONE: break;
		case PopUpWindow::PU_ERROR:
		{
			// Error
			ImGui::TextColored(ImVec4(255.f, 0.f, 0.f, 1.f), errors.empty() ? "No errors" :
				errors.c_str(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
			ImGui::Separator();

			// Solution
			ImGui::TextColored(ImVec4(0.f, 255.f, 0.f, 1.f), solutions.empty() ? "No solutions" :
				solutions.c_str(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
			ImGui::Separator();

			// Accept Button
			if (ImGui::Button(btn_text.c_str()))
			{
				active = false;
				state = PU_NONE;
				RE_EDITOR->PopUpFocus(false);
				ClearScope();
			}

			// Logs
			if (ImGui::TreeNode("Show All Logs"))
			{
				ImGui::TextEx(logs.c_str(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
				ImGui::TreePop();
			}

			// Warnings
			if (ImGui::TreeNode("Show Warnings"))
			{
				ImGui::TextEx(warnings.empty() ? "No warnings" :
					warnings.c_str(), nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
				ImGui::TreePop();
			}

			break;
		}
		case PopUpWindow::PU_SAVE_SCENE:
		{
			if (input_name)
			{
				char name_holder[64];
				EA::StdC::Snprintf(name_holder, 64, "%s", name_str.c_str());
				if (ImGui::InputText("Name", name_holder, 64)) name_str = name_holder;
			}

			bool clicked = false;

			if (ImGui::Button(btn_text.c_str()))
			{
				RE_SCENE->SaveScene((input_name) ? name_str.c_str() : nullptr);
				clicked = true;
			}

			if (ImGui::Button("Cancel")) clicked = true;

			if (clicked)
			{
				active = false;
				state = PU_NONE;
				input_name = false;
				RE_EDITOR->PopUpFocus(false);
				if (exit_after) RE_INPUT->Push(RE_EventType::REQUEST_QUIT, App);
				else if (spawn_new_scene) RE_SCENE->NewEmptyScene();
				spawn_new_scene = false;
			}

			break;
		}
		case PopUpWindow::PU_SAVE_PARTICLEEMITTER:
		{
			bool exists_emitter = false, exists_emissor = false, exists_renderer = false;
			static eastl::string emitter_path, emission_path, render_path;
			if (particle_names)
			{
				if (need_name) {
					ImGui::InputText("Emitter Name", &name_str);

					emitter_path = "Assets/Particles/";
					emitter_path += name_str;
					emitter_path += ".meta";

					exists_emitter = RE_FS->Exists(emitter_path.c_str());
					if (exists_emitter) ImGui::Text("That Emitter exists!");
				}

				if (need_emission) {
					ImGui::InputText("Emission Name", &emission_name);
				
					emission_path ="Assets/Particles/";
					emission_path += emission_name;
					emission_path += ".lasse";

					exists_emissor = RE_FS->Exists(emission_path.c_str());
					if (exists_emissor) ImGui::Text("That Emissor exists!");
				}

				if (need_renderer) {
					ImGui::InputText("Render Name", &renderer_name);
				
					render_path = "Assets/Particles/";
					render_path += renderer_name;
					render_path += ".lopfe";

					exists_renderer = RE_FS->Exists(render_path.c_str());
					if (exists_renderer) ImGui::Text("That Renderer exists!");
				}
			}

			bool clicked = false, canceled = false;

			if (exists_emitter || exists_emissor || exists_renderer)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::Button(btn_text.c_str())) clicked = true;

			if (exists_emitter || exists_emissor || exists_renderer) {
				ImGui::SameLine();
				ImGui::Text("Check names!");
			}

			if (exists_emitter || exists_emissor || exists_renderer)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}

			if (ImGui::Button("Cancel")) canceled = clicked = true;

			if (clicked)
			{
				active = false;
				state = PU_NONE;
				input_name = false;
				particle_names = false;
				need_emission = false;
				need_renderer = false;
				RE_EDITOR->PopUpFocus(false);
				if (!canceled) RE_EDITOR->SaveEmitter(exit_after, name_str.c_str(), emission_name.c_str(), renderer_name.c_str());
				else RE_EDITOR->CloseParticleEditor();
			}

			break;
		}
		case PopUpWindow::PU_PREFAB:
		{
			if (input_name)
			{
				char name_holder[64];
				EA::StdC::Snprintf(name_holder, 64, "%s", name_str.c_str());
				if (ImGui::InputText("Name", name_holder, 64)) name_str = name_holder;
			}

			static bool identityRoot = false;
			ImGui::Checkbox("Make Root Identity", &identityRoot);

			bool clicked = ImGui::Button(btn_text.c_str());
			if (clicked)
			{
				RE_Prefab* newPrefab = new RE_Prefab();
				newPrefab->SetName(name_str.c_str());
				newPrefab->SetType(Resource_Type::R_PREFAB);

				RE_INPUT->PauseEvents();
				newPrefab->Save(RE_SCENE->GetScenePool()->GetNewPoolFromID(go_prefab->GetUID()), identityRoot, true);
				RE_INPUT->ResumeEvents();

				newPrefab->SaveMeta();
				RE_RENDER->PushThumnailRend(RE_RES->Reference(newPrefab));
			}

			if (ImGui::Button("Cancel") || clicked)
			{
				state = PU_NONE;
				active = input_name = false;
				go_prefab = nullptr;
				RE_EDITOR->PopUpFocus(false);
			}

			break;
		}
		case PopUpWindow::PU_DELETERESOURCE:
		{
			ResourceContainer* res = RE_RES->At(resource_to_delete);
			ImGui::Text("Name: %s", res->GetName());

			static const char* names[MAX_R_TYPES] = { "undefined.", "shader.", "texture.", "mesh.", "prefab.", "skyBox.", "material.", "model.", "scene.", "particle emitter.", "particle emission.", "particle render." };
			ImGui::Text("Type: %s", names[res->GetType()]);

			ImGui::Separator();

			bool pushed = resource_on_scene && RE_SCENE->isPlaying();
			if (pushed)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

				ImGui::TextColored(ImVec4(255.f, 0.f, 0.f, 1.f), "Stop Scene for delete resource", nullptr, ImGuiTextFlags_NoWidthForLargeClippedText);
			}

			bool clicked = ImGui::Button(btn_text.c_str());

			if (pushed)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}

			if (clicked)
			{
				RE_LOGGER.ScopeProcedureLogging();

				// Delete at resource & filesystem
				ResourceContainer* resAlone = RE_RES->DeleteResource(resource_to_delete, using_resources, resource_on_scene);
				RE_FS->DeleteResourceFiles(resAlone);

				DEL(resAlone);
			}

			if (ImGui::Button("Cancel")) clicked = true;

			if (clicked)
			{
				active = false;
				state = PU_NONE;
				RE_EDITOR->PopUpFocus(false);
				go_prefab = nullptr;
				resource_to_delete = nullptr;
				using_resources.clear();
				resource_on_scene = false;
				RE_RES->PopSelected(true);
				RE_LOGGER.EndScope();
			}

			if (resource_on_scene) {
				ImGui::Separator();
				ImGui::Text("Your current scene will be affected.");
			}

			ImGui::Separator();
			ImGui::Text((using_resources.empty()) ? "No resources will be afected." : "The next resources will be afected and changed to default:");

			uint count = 0;
			for (auto resource : using_resources)
			{
				eastl::string btnname = eastl::to_string(count++) + ". ";
				ResourceContainer* resConflict = RE_RES->At(resource);
				static const char* names[MAX_R_TYPES] = { "Undefined | ", "Shader | ", "Texture | ", "Mesh | ", "Prefab | ", "SkyBox | ", "Material | ", "Model (need ReImport for future use) | ", "Scene | ", "Particle emitter | ", "Particle emission | ", "Particle render | " };
				btnname += (resource == RE_SCENE->GetCurrentScene()) ? "Scene (current scene) | " : names[resConflict->GetType()];
				btnname += resConflict->GetName();

				if (ImGui::Button(btnname.c_str())) RE_RES->PushSelected(resource, true);
			}

			break;
		}
		case PopUpWindow::PU_DELETEUNDEFINEDFILE:
		{
			ImGui::Text("File: %s", name_str.c_str());
			ImGui::Separator();

			bool clicked = false;
			if (ImGui::Button(btn_text.c_str()))
			{
				clicked = true;

				RE_LOGGER.ScopeProcedureLogging();
				if (!using_resources.empty())
				{
					eastl::stack<ResourceContainer*> shadersDeleted;
					for (auto resource : using_resources)
						if (RE_RES->At(resource)->GetType() == R_SHADER)
							shadersDeleted.push(RE_RES->DeleteResource(resource, RE_RES->WhereIsUsed(resource), false));

					// Delete shader files
					while (shadersDeleted.empty())
					{
						ResourceContainer* resS = shadersDeleted.top();
						RE_FS->DeleteResourceFiles(resS);
						shadersDeleted.pop();
						DEL(resS);
					}
				}

				if (!RE_LOGGER.ScopedErrors()) RE_FS->DeleteUndefinedFile(name_str.c_str());
				else RE_LOG_ERROR("File can't be erased; shaders can't be delete.");
			}

			if (ImGui::Button("Cancel")) clicked = true;

			if (clicked)
			{
				active = false;
				state = PU_NONE;
				RE_EDITOR->PopUpFocus(false);
				using_resources.clear();
				RE_RES->PopSelected(true);
				RE_LOGGER.EndScope();
			}

			ImGui::Separator();
			ImGui::Text((using_resources.empty()) ? "No resources will be afected." : "The next resources will be afected:");

			uint count = 0;
			for (auto resource : using_resources)
			{
				eastl::string btnname = eastl::to_string(count++) + ". ";
				ResourceContainer* resConflict = RE_RES->At(resource);
				Resource_Type type = resConflict->GetType();

				static const char* names[MAX_R_TYPES] = { "Undefined | ", "Shader | ", "Texture | ", "Mesh | ", "Prefab | ", "SkyBox | ", "Material | ", "Model (need ReImport for future use) | ", "Scene | ", "Particle emitter | ", "Particle emission | ", "Particle render | " };
				btnname += names[type];
				btnname += resConflict->GetName();

				if (type == R_SHADER) ImGui::Separator();
				if (ImGui::Button(btnname.c_str())) RE_RES->PushSelected(resource, true);
				if (type == R_SHADER) ImGui::Text("%s will be deleted and the next resources will be affected:", resConflict->GetName());
			}

			break;
		}
		default:
		{
			if (ImGui::Button(btn_text.c_str()))
			{
				active = false;
				state = PU_NONE;
				RE_EDITOR->PopUpFocus(false);
			}
			break;
		}
		}
	}

	ImGui::End();
}

///////   Assets Window   ////////////////////////////////////////////
AssetsWindow::AssetsWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}
AssetsWindow::~AssetsWindow() {}

const char* AssetsWindow::GetCurrentDirPath() const { return currentPath; }
void AssetsWindow::SelectUndefined(eastl::string* toFill) { selectingUndefFile = toFill; }

void AssetsWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		static RE_FileSystem::RE_Directory* currentDir = RE_FS->GetRootDirectory();
		RE_FileSystem::RE_Directory* toChange = nullptr;
		static float iconsSize = 100;

		if (ImGui::BeginMenuBar())
		{
			if (currentDir->parent == nullptr)
			{
				ImGui::Text("%s Folder", currentDir->name.c_str());
				currentPath = currentDir->path.c_str();
			}
			else
			{
				eastl::list<RE_FileSystem::RE_Directory*> folders = currentDir->FromParentToThis();
				for (auto dir : folders)
				{
					if (dir == currentDir) ImGui::Text(currentDir->name.c_str());
					else if (ImGui::Button(dir->name.c_str())) toChange = dir;

					if (dir != *folders.rbegin()) ImGui::SameLine();
				}
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::SliderFloat("Icons size", &iconsSize, 25, 256, "%.0f");
			if (selectingUndefFile)
			{
				ImGui::SameLine();
				if (ImGui::Button("Cancel selection")) selectingUndefFile = nullptr;
			}

			ImGui::EndMenuBar();
		}

		float width = ImGui::GetWindowWidth();
		int itemsColum = static_cast<int>(width / iconsSize);
		if (itemsColum == 0) itemsColum = 1;
		eastl::stack<RE_FileSystem::RE_Path*> filesToDisplay = currentDir->GetDisplayingFiles();

		ImGui::Columns(itemsColum, NULL, false);
		eastl::string idName = "#AssetImage";
		uint idCount = 0;
		while (!filesToDisplay.empty())
		{
			RE_FileSystem::RE_Path* p = filesToDisplay.top();
			filesToDisplay.pop();
			eastl::string id = idName + eastl::to_string(idCount++);
			ImGui::PushID(id.c_str());
			switch (p->pType)
			{
			case RE_FileSystem::PathType::D_FOLDER:
			{
				if (ImGui::ImageButton(reinterpret_cast<void*>(RE_EDITOR->thumbnails->GetFolderID()), { iconsSize, iconsSize }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0))
					toChange = p->AsDirectory();
				ImGui::PopID();
				ImGui::Text(p->AsDirectory()->name.c_str());
				break;
			}
			case RE_FileSystem::PathType::D_FILE:
			{
				switch (p->AsFile()->fType)
				{
				case RE_FileSystem::FileType::F_META:
				{
					ResourceContainer* res = RE_RES->At(p->AsMeta()->resource);

					unsigned int icon_meta = (res->GetType() == R_SHADER) ? RE_EDITOR->thumbnails->GetShaderFileID() : RE_EDITOR->thumbnails->GetPEmitterFileID();

					if (ImGui::ImageButton(reinterpret_cast<void*>(icon_meta), { iconsSize, iconsSize }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0))
						RE_RES->PushSelected(res->GetMD5(), true);

					if (ImGui::BeginDragDropSource())
					{
						ImGui::SetDragDropPayload((res->GetType() == R_SHADER) ? "#ShadereReference" : "#EmitterReference", &p->AsMeta()->resource, sizeof(const char**));
						ImGui::Image(reinterpret_cast<void*>(icon_meta), { 50,50 }, { 0.0f, 0.0f }, { 1.0f, 1.0f });
						ImGui::EndDragDropSource();
					}
					ImGui::PopID();

					id = idName + eastl::to_string(idCount) + "Delete";
					ImGui::PushID(id.c_str());
					if (ImGui::BeginPopupContextItem())
					{
						if (ImGui::Button("Delete")) RE_EDITOR->popupWindow->PopUpDelRes(res->GetMD5());
						ImGui::EndPopup();
					}
					ImGui::PopID();

					ImGui::Text(res->GetName());
					break;
				}
				case RE_FileSystem::FileType::F_NOTSUPPORTED:
				{
					bool pop = (!selectingUndefFile && !secondary);
					if (pop)
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
					}

					if (ImGui::ImageButton(
						reinterpret_cast<void*>(selectingUndefFile ? RE_EDITOR->thumbnails->GetSelectFileID() : RE_EDITOR->thumbnails->GetFileID()),
						{ iconsSize, iconsSize }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, (selectingUndefFile) ? -1 : 0))
					{
						if (selectingUndefFile)
						{
							*selectingUndefFile = p->path;
							selectingUndefFile = nullptr;
						}
					}
					ImGui::PopID();

					if (pop)
					{
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();
					}

					id = idName + eastl::to_string(idCount) + "Delete";
					ImGui::PushID(id.c_str());
					if (ImGui::BeginPopupContextItem())
					{
						if (ImGui::Button("Delete")) RE_EDITOR->popupWindow->PopUpDelUndeFile(p->path.c_str());
						ImGui::EndPopup();
					}
					ImGui::PopID();

					if (pop = (!selectingUndefFile && !secondary))
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
					}

					ImGui::Text(p->AsFile()->filename.c_str());

					if (pop)
					{
						ImGui::PopItemFlag();
						ImGui::PopStyleVar();
					}

					break;
				}
				default:
				{
					if (p->AsFile()->metaResource != nullptr)
					{
						ResourceContainer* res = RE_RES->At(p->AsFile()->metaResource->resource);

						unsigned int file_icon = 0;

						switch (res->GetType())
						{
						case R_PARTICLE_EMISSION: file_icon = RE_EDITOR->thumbnails->GetPEmissionFileID(); break;
						case R_PARTICLE_RENDER: file_icon = RE_EDITOR->thumbnails->GetPRenderFileID(); break;
						default: file_icon = RE_EDITOR->thumbnails->At(res->GetMD5()); break;
						}

						if (ImGui::ImageButton(reinterpret_cast<void*>(file_icon), { iconsSize, iconsSize }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0))
							RE_RES->PushSelected(res->GetMD5(), true);
						ImGui::PopID();

						id = idName + eastl::to_string(idCount) + "Delete";
						ImGui::PushID(id.c_str());
						if (ImGui::BeginPopupContextItem())
						{
							if (ImGui::Button("Delete")) RE_EDITOR->popupWindow->PopUpDelRes(res->GetMD5());
							ImGui::EndPopup();
						}
						ImGui::PopID();

						static const char* names[MAX_R_TYPES] = { "Undefined", "Shader", "Texture", "Mesh", "Prefab", "SkyBox", "Material", "Model", "Scene", "Particle emitter", "Particle emission", "Particle render" };
						eastl::string dragID("#");
						(dragID += names[res->GetType()]) += "Reference";

						if (ImGui::BeginDragDropSource())
						{
							ImGui::SetDragDropPayload(dragID.c_str(), &p->AsFile()->metaResource->resource, sizeof(const char**));
							ImGui::Image(reinterpret_cast<void*>(file_icon), { 50,50 }, { 0.0f, 0.0f }, { 1.0f, 1.0f });
							ImGui::EndDragDropSource();
						}

						ImGui::Text(p->AsFile()->filename.c_str());
					}
					else
						ImGui::PopID();

					break;
				}
				}
				break;
			}
			}

			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (toChange)
		{
			currentDir = toChange;
			currentPath = currentDir->path.c_str();
		}
	}

	ImGui::End();
}

///////   Wwise Window   ////////////////////////////////////////////
WwiseWindow::WwiseWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}
WwiseWindow::~WwiseWindow() {}

void WwiseWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		static bool usingRTPC = false;
		static bool usingState = false;
		static bool usingSwitch = false;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Send RTPC value"))
			{
				usingRTPC = true;
				usingState = usingSwitch = false;
			}
			if (ImGui::MenuItem("Send State value"))
			{
				usingState = true;
				usingRTPC = usingSwitch = false;
			}
			if (ImGui::MenuItem("Send Switch value"))
			{
				usingSwitch = true;
				usingState = usingRTPC = false;
			}
			ImGui::EndMenuBar();
		}

		if (usingRTPC || usingState || usingSwitch)
		{
			if (usingRTPC)
			{
				static eastl::string name = "RTPC_Name";
				static float value = 0;
				ImGui::InputText("Insert RTPC name", &name);
				ImGui::InputFloat("Insert RTPC value", &value);
				if (ImGui::Button("Send RTPC Value")) ModuleAudio::SendRTPC(name.c_str(), value);
			}
			else if (usingState)
			{
				static eastl::string group = "StateGroup_Name";
				static eastl::string state = "State_Name";
				ImGui::InputText("Insert State group name", &group);
				ImGui::InputText("Insert State name", &state);
				if (ImGui::Button("Send State Value")) ModuleAudio::SendState(group.c_str(), state.c_str());
			}
			else if (usingSwitch)
			{
				static eastl::string switchname = "Switch_Name";
				static eastl::string switchstate = "SwitchState_Name";
				ImGui::InputText("Insert Switch name", &switchname);
				ImGui::InputText("Insert Switch state name", &switchstate);
				if (ImGui::Button("Send Switch Value")) ModuleAudio::SendSwitch(switchname.c_str(), switchstate.c_str());
			}

			ImGui::Separator();
		}

		RE_AUDIO->DrawWwiseElementsDetected();

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
}


///////   Scene Window   ////////////////////////////////////////////
SceneEditorWindow::SceneEditorWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}
SceneEditorWindow::~SceneEditorWindow() {}

void SceneEditorWindow::UpdateViewPort()
{
	RE_CameraManager::EditorCamera()->GetTargetViewPort(viewport);
	viewport.x = (width - viewport.z) * 0.5f;
	viewport.y = (heigth - viewport.w) * 0.5f + 20;
}

void SceneEditorWindow::Recalc() { recalc = true; }

void SceneEditorWindow::Draw(bool secondary)
{
	if (need_render = ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		static int lastWidht = 0;
		static int lastHeight = 0;
		ImVec2 size = ImGui::GetWindowSize();
		width = static_cast<int>(size.x);
		heigth = static_cast<int>(size.y) - 28;
		if (recalc || lastWidht != width || lastHeight != heigth)
		{
			RE_INPUT->Push(RE_EventType::EDITORWINDOWCHANGED, RE_RENDER, RE_Cvar(lastWidht = width), RE_Cvar(lastHeight = heigth));
			RE_INPUT->Push(RE_EventType::EDITORWINDOWCHANGED, RE_EDITOR);
			recalc = false;
		}

		isWindowSelected = (ImGui::IsWindowHovered() && ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow));
		ImGui::SetCursorPos({ viewport.x, viewport.y });
		ImGui::Image((void*)RE_RENDER->GetRenderedEditorSceneTexture(), { viewport.z, viewport.w }, { 0.0, 1.0 }, { 1.0, 0.0 });

		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();
		vMin.x += ImGui::GetWindowPos().x;
		vMin.y += ImGui::GetWindowPos().y;
		vMax.x += ImGui::GetWindowPos().x;
		vMax.y += ImGui::GetWindowPos().y;

		if(!ImGuizmo::IsOver() && !ImGuizmo::IsUsing() && isWindowSelected && RE_INPUT->GetKey(SDL_SCANCODE_LALT) == KEY_IDLE && RE_INPUT->GetMouse().GetButton(1) == KEY_STATE::KEY_DOWN)
		{
			ImVec2 mousePosOnThis = ImGui::GetMousePos();
			if ((mousePosOnThis.x -= vMin.x - ImGui::GetStyle().WindowPadding.x) > 0.f && (mousePosOnThis.y -= vMin.y - ImGui::GetStyle().WindowPadding.y) > 0.f)
				RE_INPUT->Push(EDITOR_SCENE_RAYCAST, RE_EDITOR, mousePosOnThis.x, mousePosOnThis.y);
		}

		UID selected_uid;
		if (selected_uid = RE_EDITOR->GetSelected()) {
			RE_GameObject* selected = RE_SCENE->GetGOPtr(selected_uid);
			RE_GameObject* parent = selected->GetParentPtr();
			RE_CompTransform* sTransform = selected->GetTransformPtr();

			static float matA[16];
			static math::vec pos;
			static math::Quat rot, lastRot;
			static math::vec scl;

			static math::vec before = math::vec::zero, last = math::vec::zero;

			RE_CompCamera* eCamera = RE_CameraManager::EditorCamera();
			math::float4x4 cameraView = eCamera->GetView();

			bool isGlobal = false;

			//filling matA
			sTransform->GetGlobalMatrix().Transposed().Decompose(pos, rot, scl);

			static bool watchingChange = false;

			if (!watchingChange) {
				switch (operation) {
				case ImGuizmo::TRANSLATE: before = sTransform->GetLocalPosition(); break;
				case ImGuizmo::ROTATE: before = sTransform->GetLocalEulerRotation(); break;
				case ImGuizmo::SCALE: before = sTransform->GetLocalScale(); break; }
			}

			if (isGlobal = (mode == ImGuizmo::MODE::WORLD)) {
				lastRot = rot;
				rot = math::Quat::identity;
			}

			ImGuizmo::RecomposeMatrixFromComponents(pos.ptr(), rot.ToEulerXYZ().ptr(), scl.ptr(), matA);

			math::float4x4 deltamatrix = math::float4x4::identity * RE_TIME->GetDeltaTime();

			//SetRect of window at imgizmo
			ImGuizmo::SetRect(vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y);


			ImGuizmo::SetDrawlist();
			ImGuizmo::Manipulate(cameraView.ptr(), eCamera->GetProjectionPtr(), operation, mode, matA, deltamatrix.ptr());



			if (ImGuizmo::IsUsing()) {
				watchingChange = true;
				static float matrixTranslation[3], matrixRotation[3], matrixScale[3];

				ImGuizmo::DecomposeMatrixToComponents(matA, matrixTranslation, matrixRotation, matrixScale);

				math::float4x4 localMat = parent->GetTransformPtr()->GetGlobalMatrix().InverseTransposed();

		
				math::float4x4 globalMat = math::float4x4::FromTRS(math::vec(matrixTranslation), math::Quat::FromEulerXYZ(matrixRotation[0], matrixRotation[1], matrixRotation[2]), math::vec(matrixScale));
				
				localMat = localMat * globalMat;

				localMat.Decompose(pos, rot, scl);

				switch (operation) {
				case ImGuizmo::TRANSLATE: 
					sTransform->SetPosition(pos);
					last = pos;
					break;
				case ImGuizmo::ROTATE: 
					last = isGlobal ? (rot * lastRot).ToEulerXYZ() : rot.ToEulerXYZ();
					sTransform->SetRotation(last);

					break;
				case ImGuizmo::SCALE: 
					sTransform->SetScale(scl); 
					last = scl;
					break;
				}
			}


			if (watchingChange && (RE_INPUT->GetMouse().GetButton(1) == KEY_STATE::KEY_UP))
			{
				switch (operation)
				{
				case ImGuizmo::TRANSLATE:
					RE_EDITOR->PushCommand(new RE_CMDTransformPosition(selected_uid, before, last));
					break;
				case ImGuizmo::ROTATE:
					RE_EDITOR->PushCommand(new RE_CMDTransformRotation(selected_uid, before, last));
					break;
				case ImGuizmo::SCALE:
					RE_EDITOR->PushCommand(new RE_CMDTransformScale(selected_uid, before, last));
					break;
				}

				watchingChange = false; 
				before = last = math::vec::zero;
			}
		}

		if (secondary) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
}

///////   Game Window   ////////////////////////////////////////////
SceneGameWindow::SceneGameWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}
SceneGameWindow::~SceneGameWindow() {}

void SceneGameWindow::UpdateViewPort()
{
	RE_CameraManager::MainCamera()->GetTargetViewPort(viewport);
	viewport.x = (width - viewport.z) * 0.5f;
	viewport.y = (heigth - viewport.w) * 0.5f + 20;
}

void SceneGameWindow::Recalc() { recalc = true; }

void SceneGameWindow::Draw(bool secondary)
{
	if (need_render = ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		static int lastWidht = 0;
		static int lastHeight = 0;
		ImVec2 size = ImGui::GetWindowSize();
		width = static_cast<int>(size.x);
		heigth = static_cast<int>(size.y) - 28;

		if (recalc || lastWidht != width || lastHeight != heigth)
		{
			RE_INPUT->Push(RE_EventType::GAMEWINDOWCHANGED, RE_RENDER, RE_Cvar(lastWidht = width), RE_Cvar(lastHeight = heigth));
			RE_INPUT->Push(RE_EventType::GAMEWINDOWCHANGED, RE_EDITOR);
			recalc = false;
		}

		isWindowSelected = (ImGui::IsWindowHovered() && ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow));
		ImGui::SetCursorPos({ viewport.x, viewport.y });
		ImGui::Image(reinterpret_cast<void*>(RE_RENDER->GetRenderedGameSceneTexture()), { viewport.z, viewport.w }, { 0.0, 1.0 }, { 1.0, 0.0 });

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
}

///////   Debug Transforms Window   ////////////////////////////////////////////
TransformDebugWindow::TransformDebugWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}
TransformDebugWindow::~TransformDebugWindow() {}

void TransformDebugWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		eastl::vector<UID> allTransforms = RE_SCENE->GetScenePool()->GetAllGOUIDs();
		
		int transformCount = allTransforms.size();
		ImGui::Text("Total %i transforms.", transformCount);
		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_::ImGuiSeparatorFlags_Vertical);
		static int range = 0, totalShowing = 8;
		ImGui::SameLine();

		ImGui::Text("Actual Range: %i - %i", range, (range + totalShowing < transformCount) ? range + totalShowing : transformCount);

		ImGui::PushItemWidth(50.f);
		ImGui::DragInt("Position", &range, 1.f, 0, transformCount - totalShowing);
		ImGui::SameLine();
		ImGui::DragInt("List Size", &totalShowing, 1.f, 0);

		for (int i = range; i < totalShowing + range && i < transformCount; i++) {
			RE_CompTransform* transform = RE_SCENE->GetGOPtr(allTransforms[i])->GetTransformPtr();

			ImGui::PushID(("#TransformDebug" + eastl::to_string(i)).c_str());

			if (ImGui::CollapsingHeader(("GO: " + transform->GetGOPtr()->name).c_str())) {
				ImGui::Columns(2);
				static math::vec pos, rotE, scl;
				static math::float3x3 rot;

				static math::float4x4 localM;
				localM = transform->GetLocalMatrix().Transposed();
				localM.Decompose(pos, rot, scl);
				rotE = rot.ToEulerXYZ();
				ImGui::Text("Local Transform:");
				ImGui::Text("Position:");
				ImGui::Text("X %.3f | Y %.3f | Z %.3f", pos.x, pos.y, pos.z);
				ImGui::Text("Rotation:");
				ImGui::Text("X %.3f | Y %.3f | Z %.3f", math::RadToDeg(rotE.x), math::RadToDeg(rotE.y), math::RadToDeg(rotE.z));
				ImGui::Text("Scale:");
				ImGui::Text("X %.3f | Y %.3f | Z %.3f", scl.x, scl.y, scl.z);

				ImGui::NextColumn();

				static math::float4x4 globalM;
				globalM = transform->GetGlobalMatrix().Transposed();
				globalM.Decompose(pos, rot, scl);
				rotE = rot.ToEulerXYZ();
				ImGui::Text("Global Transform:");
				ImGui::Text("Position:");
				ImGui::Text("X %.3f | Y %.3f | Z %.3f", pos.x, pos.y, pos.z);
				ImGui::Text("Rotation:");
				ImGui::Text("X %.3f | Y %.3f | Z %.3f", math::RadToDeg(rotE.x), math::RadToDeg(rotE.y), math::RadToDeg(rotE.z));
				ImGui::Text("Scale:");
				ImGui::Text("X %.3f | Y %.3f | Z %.3f", scl.x, scl.y, scl.z);

				ImGui::Columns(1);
			}
			ImGui::PopID();
		}
	}
	ImGui::End();
}

RendererDebugWindow::RendererDebugWindow(const char* name, bool start_active) : EditorWindow(name, start_active) {}
RendererDebugWindow::~RendererDebugWindow() { }

void RendererDebugWindow::Draw(bool secondary)
{
	if (ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		unsigned int lightC, pLightC;
		bool sharedL;
		RE_RENDER->GetCurrentLightsCount(lightC, pLightC, sharedL);
		if (sharedL) {
			ImGui::Text("Total Lights: %u:203", lightC + pLightC);
			ImGui::Text("From lights components: %u", lightC);
			ImGui::Text("From particles: %u", pLightC);

		}
		else
		{
			ImGui::Text("Lights components: %u:203", lightC);
			ImGui::Text("Particle Lights: %u:508", pLightC);
		}
	}
	ImGui::End();
}
