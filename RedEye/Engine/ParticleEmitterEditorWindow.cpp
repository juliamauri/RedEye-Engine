#include "EditorWindow.h"
#include <EASTL/string.h>
#include <MGL/Math/float4.h>

#include "ParticleEmitterEditorWindow.h"

#include "RE_Memory.h"
#include "RE_Math.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModulePhysics.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_CameraManager.h"
#include "RE_PrimitiveManager.h"
#include "RE_CompPrimitive.h"
#include "RE_ParticleEmitter.h"
#include "RE_ParticleEmitterBase.h"
#include "PopUpWindow.h"

#define IMGUICURVEIMPLEMENTATION
#include <ImGuiWidgets/ImGuiCurverEditor/ImGuiCurveEditor.hpp>

#include <ImGuiImpl/imgui_stdlib.h>
#include <ImGui/imgui_internal.h>
#include <GL/glew.h>
#include <EASTL/bit.h>

void ParticleEmitterEditorWindow::StartEditing(RE_ParticleEmitter* sim, const char* md5)
{
	if (emiter_md5 == md5 && md5 != nullptr) return;

	if (active || simulation || emiter_md5)
	{
		if (need_save)
		{
			next_emiter_md5 = md5;
			next_simulation = sim;
			load_next = true;

			if (emiter_md5)
			{
				RE_ParticleEmitterBase* emitter = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5));
				bool has_emissor = emitter->HasEmissor(), has_render = emitter->HasRenderer();
				RE_EDITOR->popupWindow->PopUpSaveParticles((!has_emissor || !has_render), true, has_emissor, has_render);
			}
			else RE_EDITOR->popupWindow->PopUpSaveParticles(true, false, new_emitter->HasEmissor(), new_emitter->HasRenderer());

			return;
		}
		else CloseEditor();
	}

	emiter_md5 = md5;
	if (!emiter_md5) new_emitter = new RE_ParticleEmitterBase();
	active = true;
	simulation = sim;
	RE_PHYSICS->AddEmitter(simulation);
}

void ParticleEmitterEditorWindow::SaveEmitter(bool close, const char* emitter_name, const char* emissor_base, const char* renderer_base)
{
	if (emiter_md5)
	{
		RE_ParticleEmitterBase* emitter = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5));
		if (!emitter->HasEmissor() || !emitter->HasRenderer())
			emitter->GenerateSubResourcesAndReference(emissor_base, renderer_base);

		emitter->FillAndSave(simulation);
	}
	else
	{
		new_emitter->SetName(emitter_name);
		new_emitter->SetType(ResourceContainer::Type::PARTICLE_EMITTER);
		new_emitter->GenerateSubResourcesAndReference(emissor_base, renderer_base);
		new_emitter->FillAndSave(simulation);

		emiter_md5 = RE_RES->Reference(dynamic_cast<ResourceContainer*>(new_emitter));
	}

	need_save = false;

	if (load_next) LoadNextEmitter();
	else if (close) CloseEditor();
}

void ParticleEmitterEditorWindow::NextOrClose()
{
	load_next ? LoadNextEmitter() : CloseEditor();
}

void ParticleEmitterEditorWindow::CloseEditor()
{
	if (simulation) RE_PHYSICS->RemoveEmitter(simulation);
	simulation = nullptr;
	if (!emiter_md5 && new_emitter) DEL(new_emitter)
	if (emiter_md5) RE_RES->UnUse(emiter_md5);
	emiter_md5 = nullptr;
	new_emitter = nullptr;
	active = false;
}

void ParticleEmitterEditorWindow::LoadNextEmitter()
{
	CloseEditor();

	emiter_md5 = next_emiter_md5;
	if (!emiter_md5) new_emitter = new RE_ParticleEmitterBase();
	active = true;
	simulation = next_simulation;
	RE_PHYSICS->AddEmitter(simulation);

	load_next = false;
	next_emiter_md5 = nullptr;
	simulation = nullptr;
}

void ParticleEmitterEditorWindow::Draw(bool secondary)
{
	if (simulation != nullptr)
	{
		bool close = false;

		if (!docking) ImGui::GetIO().ConfigFlags -= ImGuiConfigFlags_DockingEnable;

		ImGuiWindowFlags wFlags = ImGuiWindowFlags_::ImGuiWindowFlags_None;
		wFlags |= ImGuiWindowFlags_NoCollapse;// | ImGuiWindowFlags_NoTitleBar;

		// Playback Controls
		if (ImGui::Begin("Playback Controls", NULL, wFlags | ImGuiWindowFlags_MenuBar))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::BeginMenuBar())
			{
				// Playback
				switch (simulation->state)
				{
				case RE_ParticleEmitter::PlaybackState::STOP:
				{
					if (ImGui::Button("Play simulation")) simulation->state = RE_ParticleEmitter::PlaybackState::PLAY;
					break;
				}
				case RE_ParticleEmitter::PlaybackState::PLAY:
				{
					if (ImGui::Button("Pause simulation")) simulation->state = RE_ParticleEmitter::PlaybackState::PAUSE;
					ImGui::SameLine();
					if (ImGui::Button("Stop simulation")) simulation->state = RE_ParticleEmitter::PlaybackState::STOPING;
					break;
				}
				case RE_ParticleEmitter::PlaybackState::PAUSE:
				{
					if (ImGui::Button("Resume simulation")) simulation->state = RE_ParticleEmitter::PlaybackState::PLAY;
					ImGui::SameLine();
					if (ImGui::Button("Stop simulation")) simulation->state = RE_ParticleEmitter::PlaybackState::STOPING;
					break;
				}
				}

				ImGui::SameLine();
				ImGui::Checkbox(!docking ? "Enable Docking" : "Disable Docking", &docking);


				ImGui::SameLine();
				if (ImGui::BeginMenu("Change emissor"))
				{
					eastl::vector<ResourceContainer*> meshes = RE_RES->GetResourcesByType(ResourceContainer::Type::PARTICLE_EMISSION);
					bool none = true;
					unsigned int count = 0;
					for (auto m : meshes)
					{
						if (m->isInternal()) continue;

						none = false;
						eastl::string name = eastl::to_string(count++) + m->GetName();
						if (ImGui::MenuItem(name.c_str()))
						{
							if (emiter_md5)
								dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5))->ChangeEmissor(simulation, m->GetMD5());
							else new_emitter->ChangeEmissor(simulation, m->GetMD5());

							need_save = true;
						}
					}
					if (none) ImGui::Text("No particle emissors on assets");

					ImGui::EndMenu();
				}

				ImGui::SameLine();

				if (ImGui::BeginMenu("Change render"))
				{
					eastl::vector<ResourceContainer*> meshes = RE_RES->GetResourcesByType(ResourceContainer::Type::PARTICLE_RENDER);
					bool none = true;
					unsigned int count = 0;
					for (auto m : meshes)
					{
						if (m->isInternal()) continue;

						none = false;
						eastl::string name = eastl::to_string(count++) + m->GetName();
						if (ImGui::MenuItem(name.c_str()))
						{
							if (emiter_md5)
								dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5))->ChangeRenderer(simulation, m->GetMD5());
							else new_emitter->ChangeRenderer(simulation, m->GetMD5());

							need_save = true;
						}
					}
					if (none) ImGui::Text("No particle renders on assets");

					ImGui::EndMenu();
				}

				bool disabled = false;
				if (!need_save && !secondary) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
					disabled = true;
				}

				ImGui::SameLine();
				if (ImGui::Button("Save"))
				{
					if (emiter_md5) {
						RE_ParticleEmitterBase* emitter = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5));
						bool has_emissor = emitter->HasEmissor(), has_render = emitter->HasRenderer();
						RE_EDITOR->popupWindow->PopUpSaveParticles((!has_emissor || !has_render), true, has_emissor, has_render);
					}
					else RE_EDITOR->popupWindow->PopUpSaveParticles(true, false, new_emitter->HasEmissor(), new_emitter->HasRenderer());
				}
				ImGui::SameLine();
				if (ImGui::Button("Discard changes"))
				{
					DEL(simulation)
					simulation = (emiter_md5) ? dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5))->GetNewEmitter()
						: new RE_ParticleEmitter(true);

					if (!emiter_md5)
					{
						DEL(new_emitter)
						new_emitter = new RE_ParticleEmitterBase();
					}

					need_save = false;
				}

				if (disabled)
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}

				ImGui::SameLine();

				if (ImGui::Button("Close")) close = true;
			}
			ImGui::EndMenuBar();

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		// Viewport
		if (ImGui::Begin(name, NULL, wFlags | ImGuiWindowFlags_MenuBar))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::BeginMenuBar())
			{
				static bool debug_draw = RE_RENDER->GetRenderViewDebugDraw(RenderView::Type::PARTICLE);
				static bool deferred = (RE_RENDER->GetRenderViewLightMode(RenderView::Type::PARTICLE) == RenderView::LightMode::DEFERRED);
				static math::float4 clear_color = RE_RENDER->GetRenderViewClearColor(RenderView::Type::PARTICLE);

				if (ImGui::Checkbox("Particle debug draw", &debug_draw))
					RE_RENDER->SetRenderViewDebugDraw(RenderView::Type::PARTICLE, debug_draw);

				if (ImGui::Checkbox("Deferred lighting", &deferred)) {
					RE_RENDER->SetRenderViewDeferred(RenderView::Type::PARTICLE, deferred);
					if (deferred) {
						clear_color = { 0.0f,0.0f,0.0f,1.0 };
						RE_RENDER->SetRenderViewClearColor(RenderView::Type::PARTICLE, clear_color);
					}
				}

				if (!deferred) {
					ImGui::PushItemWidth(150.0f);

					if (ImGui::ColorEdit3("Background color", clear_color.ptr()))
						RE_RENDER->SetRenderViewClearColor(RenderView::Type::PARTICLE, clear_color);

					ImGui::PopItemWidth();
				}

			}
			ImGui::EndMenuBar();

			static int lastWidht = 0;
			static int lastHeight = 0;
			ImVec2 size = ImGui::GetWindowSize();
			width = static_cast<int>(size.x);
			heigth = static_cast<int>(size.y) - 28;
			if (recalc || lastWidht != width || lastHeight != heigth)
			{
				RE_INPUT->Push(RE_EventType::PARTRICLEEDITORWINDOWCHANGED, RE_RENDER, RE_Cvar(lastWidht = width), RE_Cvar(lastHeight = heigth));
				RE_INPUT->Push(RE_EventType::PARTRICLEEDITORWINDOWCHANGED, RE_EDITOR);
				recalc = false;
			}

			isWindowSelected = (ImGui::IsWindowHovered() && ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow));
			ImGui::SetCursorPos({ viewport.x, viewport.y });
			ImGui::Image(eastl::bit_cast<void*>(RE_RENDER->GetRenderViewTexture(RenderView::Type::PARTICLE)), { viewport.z, viewport.w }, { 0.0, 1.0 }, { 1.0, 0.0 });
			
			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Status", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			// Control (read-only)
			ImGui::Text("Current particles: %i", simulation->particle_count);
			ImGui::Text("Total time: %.1f s", simulation->total_time);
			ImGui::Text("Max Distance: %.1f units", math::SqrtFast(simulation->max_dist_sq));
			ImGui::Text("Max Speed: %.1f units/s", math::SqrtFast(simulation->max_speed_sq));
			ImGui::Text("Parent Position: %.1f, %.1f, %.1f", simulation->parent_pos.x, simulation->parent_pos.y, simulation->parent_pos.z);
			ImGui::Text("Parent Speed: %.1f, %.1f, %.1f", simulation->parent_speed.x, simulation->parent_speed.y, simulation->parent_speed.z);

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Spawning", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			int tmp = static_cast<int>(simulation->max_particles);
			if (ImGui::DragInt("Max particles", &tmp, 1.f, 0, 65000))
			{
				simulation->max_particles = static_cast<unsigned int>(RE_Math::Cap(tmp, 0, 500000));
				need_save = true;
			}

			if (simulation->spawn_interval.DrawEditor(need_save) + simulation->spawn_mode.DrawEditor(need_save) &&
				simulation->state != RE_ParticleEmitter::PlaybackState::STOP)
					simulation->state = RE_ParticleEmitter::PlaybackState::RESTART;

			ImGui::Separator();
			if (ImGui::Checkbox("Start on Play", &simulation->start_on_play))  need_save = true;
			if (ImGui::Checkbox("Loop", &simulation->loop))  need_save = true;
			if (!simulation->loop)
			{
				ImGui::SameLine();
				if (ImGui::DragFloat("Max time", &simulation->max_time, 1.f, 0.f, 10000.f))
					need_save = true;
			}
			if (ImGui::DragFloat("Time Multiplier", &simulation->time_muliplier, 0.01f, 0.01f, 10.f)) need_save = true;
			if (ImGui::DragFloat("Start Delay", &simulation->start_delay, 1.f, 0.f, 10000.f)) need_save = true;

			ImGui::Separator();
			if (ImGui::Checkbox("Local Space", &simulation->local_space)) need_save = true;
			if (ImGui::Checkbox("Inherit Speed", &simulation->inherit_speed)) need_save = true;

			ImGui::Separator();
			if (simulation->initial_lifetime.DrawEditor("Lifetime")) need_save = true;

			ImGui::Separator();
			if (simulation->initial_pos.DrawEditor()) need_save = true;

			ImGui::Separator();
			if (simulation->initial_speed.DrawEditor("Starting Speed")) need_save = true;

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Physics", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (simulation->external_acc.DrawEditor()) need_save = true;

			ImGui::Separator();
			if (simulation->boundary.DrawEditor()) need_save = true;

			ImGui::Separator();
			if (simulation->collider.DrawEditor()) need_save = true;

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Particle Shape", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (simulation->meshMD5)
			{
				if (ImGui::Button(eastl::string("Resource Mesh").c_str()))
					RE_RES->PushSelected(simulation->meshMD5, true);
			}
			else if (!simulation->primCmp)
				ImGui::TextWrapped("Select mesh resource or select primitive");
			else if (simulation->primCmp->DrawPrimPropierties()) need_save = true;
			 

			ImGui::Separator();

			static bool clearMesh = false, setUpPrimitive = false;
			if (ImGui::BeginMenu("Primitive"))
			{
				if (ImGui::MenuItem("Point"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompPoint();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Cube"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompCube();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Dodecahedron"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompDodecahedron();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Tetrahedron"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompTetrahedron();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Octohedron"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompOctohedron();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Icosahedron"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompIcosahedron();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Plane"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompPlane();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Sphere"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompSphere();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Cylinder"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompCylinder();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("HemiSphere"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompHemiSphere();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Torus"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompTorus();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Trefoil Knot"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompTrefoiKnot();
					setUpPrimitive = clearMesh = true;
				}
				if (ImGui::MenuItem("Rock"))
				{
					if (simulation->primCmp)
					{
						simulation->primCmp->UnUseResources();
						DEL(simulation->primCmp)
					}
					simulation->primCmp = new RE_CompRock();
					setUpPrimitive = clearMesh = true;
				}

				ImGui::EndMenu();
			}

			if (clearMesh)
			{
				need_save = true;

				if (simulation->meshMD5)
				{
					RE_RES->UnUse(simulation->meshMD5);
					simulation->meshMD5 = nullptr;
				}

				if (setUpPrimitive)
				{
					RE_SCENE->primitives->SetUpComponentPrimitive(simulation->primCmp);
					setUpPrimitive = false;
				}

				clearMesh = false;
			}

			if (ImGui::BeginMenu("Change mesh"))
			{
				eastl::vector<ResourceContainer*> meshes = RE_RES->GetResourcesByType(ResourceContainer::Type::MESH);
				bool none = true;
				unsigned int count = 0;
				for (auto m : meshes)
				{
					if (m->isInternal()) continue;

					none = false;
					eastl::string name = eastl::to_string(count++) + m->GetName();
					if (ImGui::MenuItem(name.c_str()))
					{
						if (simulation->meshMD5) RE_RES->UnUse(simulation->meshMD5);
						simulation->meshMD5 = m->GetMD5();
						if (simulation->meshMD5) RE_RES->Use(simulation->meshMD5);

						need_save = true;
					}
				}
				if (none) ImGui::Text("No meshes on assets");

				ImGui::EndMenu();
			}
			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Particle Lighting", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (simulation->light.DrawEditor(simulation->id)) need_save = true;

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Particle Renderer Settings", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::DragFloat3("Scale", simulation->scale.ptr(), 0.1f, -10000.f, 10000.f, "%.2f"))
			{
				if (!simulation->scale.IsFinite())simulation->scale.Set(0.5f, 0.5f, 0.5f);
				need_save = true;
			}

			int pDir = static_cast<int>(simulation->orientation);
			if (ImGui::Combo("Particle Direction", &pDir, "Normal\0Billboard\0Custom\0"))
			{
				simulation->orientation = static_cast<RE_ParticleEmitter::ParticleDir>(pDir);
				need_save = true;
			}

			if (simulation->orientation == RE_ParticleEmitter::ParticleDir::Custom)
			{
				ImGui::DragFloat3("Custom Direction", simulation->direction.ptr(), 0.1f, -1.f, 1.f, "%.2f");
				need_save = true;
			}

			if (simulation->color.DrawEditor()) need_save = true;
			if (simulation->opacity.DrawEditor()) need_save = true;

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Opacity Curve", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (simulation->opacity.type != RE_PR_Opacity::Type::VALUE && simulation->opacity.type != RE_PR_Opacity::Type::NONE && simulation->opacity.useCurve)
			{
				if (simulation->opacity.curve.DrawEditor("Opacity")) need_save = true;
			}
			else
				ImGui::Text("Select a opacity over type and enable curve at opacity propierties.");

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		if (ImGui::Begin("Color Curve", NULL, wFlags))
		{
			if (secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (simulation->color.type != RE_PR_Color::Type::SINGLE && simulation->color.useCurve)
			{
				if (simulation->color.curve.DrawEditor("Color")) need_save = true;
			}
			else
				ImGui::Text("Select a color over type and enable curve at color propierties.");

			if (secondary)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::End();

		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		if (close)
		{
			if (need_save)
			{
				if (emiter_md5)
				{
					RE_ParticleEmitterBase* emitter = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5));
					bool has_emissor = emitter->HasEmissor(), has_render = emitter->HasRenderer();
					RE_EDITOR->popupWindow->PopUpSaveParticles((!has_emissor || !has_render), true, has_emissor, has_render, true);
				}
				else RE_EDITOR->popupWindow->PopUpSaveParticles(true, false, new_emitter->HasEmissor(), new_emitter->HasRenderer(), true);
			}
			else CloseEditor();
		}
	}
}
