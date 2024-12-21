#include "Event.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>

#include "PopUpWindow.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_Prefab.h"
#include "RE_ParticleEmitterBase.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <ImGuiImpl/imgui_stdlib.h>
#include <EAStdC/EASprintf.h>

namespace PopUpFlags {
	constexpr unsigned char
		none = 0,
		exit_after = 1 << 0,
		input_name = 1 << 1,
		spawn_new_scene = 1 << 2,
		particle_names = 1 << 3,
		need_name = 1 << 4,
		need_emission = 1 << 5,
		need_renderer = 1 << 6,
		resource_on_scene = 1 << 7;
}

void PopUpWindow::PopUp(const char* _btnText, const char* title, bool _disableAllWindows)
{
	btn_text = _btnText;
	title_text = title;
	RE_EDITOR->PopUpFocus(_disableAllWindows);
	active = true;
}

void PopUpWindow::PopUpError()
{
	state = PopUpState::ERROR;
	PopUp("Accept", "Error", true);
}

void PopUpWindow::PopUpSaveScene(bool fromExit, bool newScene)
{
	state = PopUpState::SAVE_SCENE;
	if (fromExit) flags |= PopUpFlags::exit_after;
	if (newScene) flags |= PopUpFlags::spawn_new_scene;

	if (RE_SCENE->isNewScene())
	{
		flags |= PopUpFlags::input_name;
		name_str = "New Scene";
	}

	PopUp("Save", "Scene have changes", true);
}

void PopUpWindow::PopUpSaveParticles(bool need_particle_names, bool not_name, bool not_emissor, bool not_renderer, bool close_after)
{
	state = PopUpState::SAVE_PARTICLE_EMITTER;
	if (close_after) flags |= PopUpFlags::exit_after;
	if (need_particle_names)
	{
		name_str = "emitter_name";
		emission_name = "emission_name";
		renderer_name = "renderer_name";

		flags |= PopUpFlags::particle_names;
		if (!not_name) flags |= PopUpFlags::need_name;
		if (!not_emissor) flags |= PopUpFlags::need_emission;
		if (!not_renderer) flags |= PopUpFlags::need_renderer;
	}

	PopUp("Save", "Save particle emittter", true);
}

void PopUpWindow::PopUpPrefab(RE_GameObject* go)
{
	state = PopUpState::PREFAB;
	flags |= PopUpFlags::input_name;
	name_str = "New Prefab";
	go_prefab = go;
	PopUp("Save", "Create prefab", false);
}

void PopUpWindow::PopUpDelRes(const char* res)
{
	//TODO PARTICLE RESOUCES
	state = PopUpState::DELETE_RESOURCE;
	resource_to_delete = res;
	using_resources = RE_RES->WhereIsUsed(res);

	eastl::stack<RE_Component*> comps;
	ResourceContainer::Type rType = RE_RES->At(res)->GetType();

	switch (rType)
	{
	case ResourceContainer::Type::SKYBOX:
		comps = RE_SCENE->GetScenePool()->GetRootPtr()->GetAllChildsComponents(RE_Component::Type::CAMERA);
		break;
	case ResourceContainer::Type::MATERIAL:
		comps = RE_SCENE->GetScenePool()->GetRootPtr()->GetAllChildsComponents(RE_Component::Type::MESH);
		break;
	case ResourceContainer::Type::PARTICLE_EMISSION:
	case ResourceContainer::Type::PARTICLE_RENDER:
	case ResourceContainer::Type::PARTICLE_EMITTER:
		comps = RE_SCENE->GetScenePool()->GetRootPtr()->GetAllChildsComponents(RE_Component::Type::PARTICLEEMITER);
		break;
	default: break;
	}

	bool skip = false;
	while (!comps.empty() && !skip)
	{
		switch (rType)
		{
		case ResourceContainer::Type::SKYBOX:
		{
			RE_CompCamera* cam = dynamic_cast<RE_CompCamera*>(comps.top());
			if (cam && cam->GetSkybox() == res)
			{
				flags |= PopUpFlags::resource_on_scene;
				skip = true;
			}
			break;
		}
		case ResourceContainer::Type::MATERIAL:
		{
			RE_CompMesh* mesh = dynamic_cast<RE_CompMesh*>(comps.top());
			if (mesh && mesh->GetMaterial() == res)
			{
				flags |= PopUpFlags::resource_on_scene;
				skip = true;
			}
			break;
		}
		case ResourceContainer::Type::PARTICLE_EMITTER:
		{
			RE_CompParticleEmitter* emitter = dynamic_cast<RE_CompParticleEmitter*>(comps.top());
			if (emitter && emitter->GetEmitterResource() == res)
			{
				flags |= PopUpFlags::resource_on_scene;
				skip = true;
			}
			break;
		}
		case ResourceContainer::Type::PARTICLE_EMISSION:
		case ResourceContainer::Type::PARTICLE_RENDER:
		{
			RE_CompParticleEmitter* emitter = dynamic_cast<RE_CompParticleEmitter*>(comps.top());
			if (emitter)
			{
				const char* emitter_res = emitter->GetEmitterResource();
				if (emitter_res)
				{
					if (dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emitter_res))->Contains(res))
					{
						flags |= PopUpFlags::resource_on_scene;
						skip = true;
					}
				}
			}
			break;
		}
		default: break;
		}

		comps.pop();
	}

	PopUp("Delete", "Do you want to delete that resource?", false);
}

void PopUpWindow::PopUpDelUndeFile(const char* assetPath)
{
	state = PopUpState::DELETE_UNDEFINED_FILE;
	name_str = assetPath;
	using_resources = RE_RES->WhereUndefinedFileIsUsed(assetPath);
	PopUp("Delete", "Do you want to delete that file?", false);
}

void PopUpWindow::AppendScopedLog(const char* log, RE_EventType type)
{
	if (log != nullptr)
	{
		logs += log;
		switch (type) {
		case RE_EventType::CONSOLE_LOG_SAVE_ERROR: errors += log; break;
		case RE_EventType::CONSOLE_LOG_SAVE_WARNING: warnings += log; break;
		case RE_EventType::CONSOLE_LOG_SAVE_SOLUTION: solutions += log; break;
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
	if (ImGui::Begin(title_text.c_str(), 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{
		switch (state)
		{
		case PopUpState::NONE: break;
		case PopUpState::ERROR:
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
				state = PopUpState::NONE;
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
		case PopUpState::SAVE_SCENE:
		{
			if (flags & PopUpFlags::input_name)
			{
				char name_holder[64];
				EA::StdC::Snprintf(name_holder, 64, "%s", name_str.c_str());
				if (ImGui::InputText("Name", name_holder, 64)) name_str = name_holder;
			}

			bool clicked = false;

			if (ImGui::Button(btn_text.c_str()))
			{
				RE_SCENE->SaveScene((flags & PopUpFlags::input_name) ? name_str.c_str() : nullptr);
				clicked = true;
			}

			if (ImGui::Button("Cancel")) clicked = true;

			if (clicked)
			{
				active = false;
				state = PopUpState::NONE;
				flags &= ~PopUpFlags::input_name;
				RE_EDITOR->PopUpFocus(false);
				if (flags & PopUpFlags::exit_after) RE_INPUT->Push(RE_EventType::REQUEST_QUIT, App);
				else if (flags & PopUpFlags::spawn_new_scene) RE_SCENE->NewEmptyScene();
				flags &= ~PopUpFlags::spawn_new_scene;
			}

			break;
		}
		case PopUpState::SAVE_PARTICLE_EMITTER:
		{
			bool exists_emitter = false, exists_emissor = false, exists_renderer = false;
			static eastl::string emitter_path, emission_path, render_path;
			if (flags & PopUpFlags::particle_names)
			{
				if (flags & PopUpFlags::need_name)
				{
					ImGui::InputText("Emitter Name", &name_str);

					emitter_path = "Assets/Particles/";
					emitter_path += name_str;
					emitter_path += ".meta";

					exists_emitter = RE_FS->Exists(emitter_path.c_str());
					if (exists_emitter) ImGui::Text("That Emitter exists!");
				}

				if (flags & PopUpFlags::need_emission)
				{
					ImGui::InputText("Emission Name", &emission_name);

					emission_path = "Assets/Particles/";
					emission_path += emission_name;
					emission_path += ".lasse";

					exists_emissor = RE_FS->Exists(emission_path.c_str());
					if (exists_emissor) ImGui::Text("That Emissor exists!");
				}

				if (flags & PopUpFlags::need_renderer)
				{
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
				state = PopUpState::NONE;
				flags &= ~(PopUpFlags::input_name | PopUpFlags::particle_names | PopUpFlags::need_emission | PopUpFlags::need_renderer);
				RE_EDITOR->PopUpFocus(false);
				if (!canceled) RE_EDITOR->SaveEmitter(flags & PopUpFlags::exit_after, name_str.c_str(), emission_name.c_str(), renderer_name.c_str());
				else RE_EDITOR->CloseParticleEditor();
			}

			break;
		}
		case PopUpState::PREFAB:
		{
			if (flags & PopUpFlags::input_name)
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
				newPrefab->SetType(ResourceContainer::Type::PREFAB);

				RE_INPUT->PauseEvents();
				newPrefab->Save(RE_SCENE->GetScenePool()->GetNewPoolFromID(go_prefab->GetUID()), identityRoot, true);
				RE_INPUT->ResumeEvents();

				newPrefab->SaveMeta();
				RE_RENDER->PushThumnailRend(RE_RES->Reference(newPrefab));
			}

			if (ImGui::Button("Cancel") || clicked)
			{
				state = PopUpState::NONE;
				active = false;
				flags &= ~PopUpFlags::input_name;
				go_prefab = nullptr;
				RE_EDITOR->PopUpFocus(false);
			}

			break;
		}
		case PopUpState::DELETE_RESOURCE:
		{
			ResourceContainer* res = RE_RES->At(resource_to_delete);
			ImGui::Text("Name: %s", res->GetName());

			static const char* names[static_cast<ushort>(ResourceContainer::Type::MAX)] =
			{ "undefined.", "shader.", "texture.", "mesh.", "prefab.", "skyBox.", "material.", "model.", "scene.", "particle emitter.", "particle emission.", "particle render." };
			ImGui::Text("Type: %s", names[static_cast<ushort>(res->GetType())]);

			ImGui::Separator();

			bool pushed = (flags & PopUpFlags::resource_on_scene) && RE_SCENE->isPlaying();
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
				RE_LOGGER::ScopeProcedureLogging();

				// Delete at resource & filesystem
				ResourceContainer* resAlone = RE_RES->DeleteResource(resource_to_delete, using_resources, flags & PopUpFlags::resource_on_scene);
				RE_FS->DeleteResourceFiles(resAlone);

				DEL(resAlone)
			}

			if (ImGui::Button("Cancel")) clicked = true;

			if (clicked)
			{
				active = false;
				state = PopUpState::NONE;
				RE_EDITOR->PopUpFocus(false);
				go_prefab = nullptr;
				resource_to_delete = nullptr;
				using_resources.clear();
				flags &= ~PopUpFlags::resource_on_scene;
				RE_RES->PopSelected(true);
				RE_LOGGER::EndScope();
			}

			if (flags & PopUpFlags::resource_on_scene)
			{
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
				static const char* names[static_cast<ushort>(ResourceContainer::Type::MAX)] =
				{ "Undefined | ", "Shader | ", "Texture | ", "Mesh | ", "Prefab | ", "SkyBox | ", "Material | ", "Model (need ReImport for future use) | ", "Scene | ", "Particle emitter | ", "Particle emission | ", "Particle render | " };

				btnname += (resource == RE_SCENE->GetCurrentScene()) ? "Scene (current scene) | " : names[static_cast<ushort>(resConflict->GetType())];
				btnname += resConflict->GetName();

				if (ImGui::Button(btnname.c_str())) RE_RES->PushSelected(resource, true);
			}

			break;
		}
		case PopUpState::DELETE_UNDEFINED_FILE:
		{
			ImGui::Text("File: %s", name_str.c_str());
			ImGui::Separator();

			bool clicked = false;
			if (ImGui::Button(btn_text.c_str()))
			{
				clicked = true;

				RE_LOGGER::ScopeProcedureLogging();
				if (!using_resources.empty())
				{
					eastl::stack<ResourceContainer*> shadersDeleted;
					for (auto resource : using_resources)
						if (RE_RES->At(resource)->GetType() == ResourceContainer::Type::SHADER)
							shadersDeleted.push(RE_RES->DeleteResource(resource, RE_RES->WhereIsUsed(resource), false));

					// Delete shader files
					while (shadersDeleted.empty())
					{
						ResourceContainer* resS = shadersDeleted.top();
						RE_FS->DeleteResourceFiles(resS);
						shadersDeleted.pop();
						DEL(resS)
					}
				}

				if (!RE_LOGGER::ScopedErrors()) RE_FS->DeleteUndefinedFile(name_str.c_str());
				else RE_LOG_ERROR("File can't be erased; shaders can't be delete.");
			}

			if (ImGui::Button("Cancel")) clicked = true;

			if (clicked)
			{
				active = false;
				state = PopUpState::NONE;
				RE_EDITOR->PopUpFocus(false);
				using_resources.clear();
				RE_RES->PopSelected(true);
				RE_LOGGER::EndScope();
			}

			ImGui::Separator();
			ImGui::Text((using_resources.empty()) ? "No resources will be afected." : "The next resources will be afected:");

			uint count = 0;
			for (auto resource : using_resources)
			{
				eastl::string btnname = eastl::to_string(count++) + ". ";
				ResourceContainer* resConflict = RE_RES->At(resource);
				ResourceContainer::Type type = resConflict->GetType();

				static const char* names[static_cast<ushort>(ResourceContainer::Type::MAX)] =
				{ "Undefined | ", "Shader | ", "Texture | ", "Mesh | ", "Prefab | ", "SkyBox | ", "Material | ", "Model (need ReImport for future use) | ", "Scene | ", "Particle emitter | ", "Particle emission | ", "Particle render | " };
				
				btnname += names[static_cast<ushort>(type)];
				btnname += resConflict->GetName();

				if (type == ResourceContainer::Type::SHADER) ImGui::Separator();
				if (ImGui::Button(btnname.c_str())) RE_RES->PushSelected(resource, true);
				if (type == ResourceContainer::Type::SHADER)
					ImGui::Text("%s will be deleted and the next resources will be affected:", resConflict->GetName());
			}

			break;
		}
		default:
		{
			if (ImGui::Button(btn_text.c_str()))
			{
				active = false;
				state = PopUpState::NONE;
				RE_EDITOR->PopUpFocus(false);
			}
			break;
		}
		}
	}

	ImGui::End();
}