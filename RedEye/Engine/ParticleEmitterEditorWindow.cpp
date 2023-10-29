#include "ParticleEmitterEditorWindow.h"

#include "RE_Memory.h"
#include "RE_Math.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "PopUpWindow.h"

#include "RE_FBOManager.h"
#include "RE_ResourceManager.h"
#include "RE_CameraManager.h"
#include "RE_PrimitiveManager.h"
#include "RE_ParticleManager.h"

#include "RE_CompPrimitive.h"
#include "RE_ParticleEmitter.h"
#include "RE_ParticleEmitterBase.h"

#define IMGUICURVEIMPLEMENTATION
#include <ImGuiWidgets/ImGuiCurverEditor/ImGuiCurveEditor.hpp>

#include <ImGuiImpl/imgui_stdlib.h>
#include <ImGui/imgui_internal.h>
#include <GL/glew.h>
#include <EASTL/bit.h>

ParticleEmitterEditorWindow::ParticleEmitterEditorWindow() :
	OwnCameraRenderedWindow("Particle Emitter Workspace", false)
{
	render_view.settings.flags =
		RenderSettings::Flag::FACE_CULLING |
		RenderSettings::Flag::TEXTURE_2D |
		RenderSettings::Flag::COLOR_MATERIAL |
		//RenderSettings::Flag::DEPTH_TEST |
		//RenderSettings::Flag::CLIP_DISTANCE |

		RenderSettings::Flag::DEBUG_DRAW |
		RenderSettings::Flag::BLENDED;

	render_view.settings.light = RenderSettings::LightMode::DISABLED;
	render_view.fbos = {
		RE_FBOManager::CreateFBO(1024, 768, 1, true, false),
		RE_FBOManager::CreateDeferredFBO(1024, 768) };
}

void ParticleEmitterEditorWindow::RenderFBO() const
{
	if (!active || !need_render) return;

	RE_RENDER->DrawParticleEmitter(simulation, render_view, cam);
}

void ParticleEmitterEditorWindow::Orbit(float delta_x, float delta_y)
{
	if (!simulation) return;

	RE_ParticleEmitter* emitter = RE_ParticleManager::GetEmitter(simulation);
	if (emitter) cam.Orbit(
		cam_sensitivity * -delta_x,
		cam_sensitivity * delta_y,
		emitter->bounding_box.CenterPoint());
}

void ParticleEmitterEditorWindow::Focus()
{
	if (!simulation) return;

	RE_ParticleEmitter* emitter = RE_ParticleManager::GetEmitter(simulation);
	if (!emitter) return;

	math::AABB box = emitter->bounding_box;
	cam.Focus(box.CenterPoint(), box.HalfSize().Length());
}

void ParticleEmitterEditorWindow::StartEditing(P_UID sim, const char* md5)
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
}

void ParticleEmitterEditorWindow::SaveEmitter(
	bool close,
	const char* emitter_name,
	const char* emissor_base,
	const char* renderer_base)
{
	RE_ParticleEmitter* emitter = RE_ParticleManager::GetEmitter(simulation);
	if (!emitter) return;

	if (emiter_md5)
	{
		auto emitter_base = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5));
		if (!emitter_base->HasEmissor() || !emitter_base->HasRenderer())
			emitter_base->GenerateSubResourcesAndReference(emissor_base, renderer_base);

		emitter_base->FillAndSave(emitter);
	}
	else
	{
		new_emitter->SetName(emitter_name);
		new_emitter->SetType(ResourceContainer::Type::PARTICLE_EMITTER);
		new_emitter->GenerateSubResourcesAndReference(emissor_base, renderer_base);
		new_emitter->FillAndSave(emitter);

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
	RE_ParticleManager::Deallocate(simulation);
	simulation = 0;
	if (emiter_md5) RE_RES->UnUse(emiter_md5);
	emiter_md5 = nullptr;
	DEL(new_emitter)
	active = false;
}

void ParticleEmitterEditorWindow::LoadNextEmitter()
{
	CloseEditor();

	emiter_md5 = next_emiter_md5;
	if (!emiter_md5) new_emitter = new RE_ParticleEmitterBase();
	active = true;
	simulation = next_simulation;
	RE_ParticleManager::Deallocate(simulation);

	load_next = false;
	next_emiter_md5 = nullptr;
	simulation = 0;
}

void ParticleEmitterEditorWindow::Draw(bool secondary)
{
	if (!simulation) return;

	RE_ParticleEmitter* emitter = RE_ParticleManager::GetEmitter(simulation);
	if (!emitter) return;

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
			switch (emitter->state)
			{
			case RE_ParticleEmitter::PlaybackState::STOP:
			{
				if (ImGui::Button("Play simulation")) emitter->state = RE_ParticleEmitter::PlaybackState::PLAY;
				break;
			}
			case RE_ParticleEmitter::PlaybackState::PLAY:
			{
				if (ImGui::Button("Pause simulation")) emitter->state = RE_ParticleEmitter::PlaybackState::PAUSE;
				ImGui::SameLine();
				if (ImGui::Button("Stop simulation")) emitter->state = RE_ParticleEmitter::PlaybackState::STOPING;
				break;
			}
			case RE_ParticleEmitter::PlaybackState::PAUSE:
			{
				if (ImGui::Button("Resume simulation")) emitter->state = RE_ParticleEmitter::PlaybackState::PLAY;
				ImGui::SameLine();
				if (ImGui::Button("Stop simulation")) emitter->state = RE_ParticleEmitter::PlaybackState::STOPING;
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
							dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5))->ChangeEmissor(emitter, m->GetMD5());
						else new_emitter->ChangeEmissor(emitter, m->GetMD5());

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
							dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5))->ChangeRenderer(emitter, m->GetMD5());
						else new_emitter->ChangeRenderer(emitter, m->GetMD5());

						need_save = true;
					}
				}
				if (none) ImGui::Text("No particle renders on assets");

				ImGui::EndMenu();
			}

			bool disabled = false;
			if (!need_save && !secondary)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				disabled = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("Save"))
			{
				if (emiter_md5)
				{
					RE_ParticleEmitterBase* emitter = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5));
					bool has_emissor = emitter->HasEmissor(), has_render = emitter->HasRenderer();
					RE_EDITOR->popupWindow->PopUpSaveParticles((!has_emissor || !has_render), true, has_emissor, has_render);
				}
				else RE_EDITOR->popupWindow->PopUpSaveParticles(true, false, new_emitter->HasEmissor(), new_emitter->HasRenderer());
			}
			ImGui::SameLine();
			if (ImGui::Button("Discard changes"))
			{
				RE_ParticleManager::Deallocate(simulation);
				simulation = emiter_md5 ?
					RE_ParticleManager::Allocate(*dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5))->GetNewEmitter()) :
					RE_ParticleManager::Allocate(*new RE_ParticleEmitter(true));

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
			render_view.settings.CheckboxFlag("Particle debug draw", RenderSettings::Flag::DEBUG_DRAW);

			bool deferred = (render_view.settings.light == RenderSettings::LightMode::DEFERRED);
			if (ImGui::Checkbox("Deferred lighting", &deferred))
			{
				if (deferred)
				{
					render_view.settings.light = RenderSettings::LightMode::DEFERRED;
					render_view.clear_color = { 0.0f,0.0f,0.0f,1.0 };
				}
				else render_view.settings.light = RenderSettings::LightMode::DISABLED;
			}

			if (!deferred)
			{
				math::float4 clear_color = render_view.clear_color;
				ImGui::PushItemWidth(150.0f);
				if (ImGui::ColorEdit3("Background color", clear_color.ptr()))
					render_view.clear_color = clear_color;
				ImGui::PopItemWidth();
			}
		}
		ImGui::EndMenuBar();

		UpdateWindow();

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
		ImGui::Text("Current particles: %i", emitter->particle_count);
		ImGui::Text("Total time: %.1f s", emitter->total_time);
		ImGui::Text("Max Distance: %.1f units", math::SqrtFast(emitter->max_dist_sq));
		ImGui::Text("Max Speed: %.1f units/s", math::SqrtFast(emitter->max_speed_sq));
		ImGui::Text("Parent Position: %.1f, %.1f, %.1f", emitter->parent_pos.x, emitter->parent_pos.y, emitter->parent_pos.z);
		ImGui::Text("Parent Speed: %.1f, %.1f, %.1f", emitter->parent_speed.x, emitter->parent_speed.y, emitter->parent_speed.z);

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

		int tmp = static_cast<int>(emitter->max_particles);
		if (ImGui::DragInt("Max particles", &tmp, 1.f, 0, 65000))
		{
			emitter->max_particles = static_cast<unsigned int>(RE_Math::Cap(tmp, 0, 500000));
			need_save = true;
		}

		if (emitter->spawn_interval.DrawEditor(need_save) + emitter->spawn_mode.DrawEditor(need_save) &&
			emitter->state != RE_ParticleEmitter::PlaybackState::STOP)
			emitter->state = RE_ParticleEmitter::PlaybackState::RESTART;

		ImGui::Separator();
		if (ImGui::Checkbox("Start on Play", &emitter->start_on_play))  need_save = true;
		if (ImGui::Checkbox("Loop", &emitter->loop))  need_save = true;
		if (!emitter->loop)
		{
			ImGui::SameLine();
			if (ImGui::DragFloat("Max time", &emitter->max_time, 1.f, 0.f, 10000.f))
				need_save = true;
		}
		if (ImGui::DragFloat("Time Multiplier", &emitter->time_muliplier, 0.01f, 0.01f, 10.f)) need_save = true;
		if (ImGui::DragFloat("Start Delay", &emitter->start_delay, 1.f, 0.f, 10000.f)) need_save = true;

		ImGui::Separator();
		if (ImGui::Checkbox("Local Space", &emitter->local_space)) need_save = true;
		if (ImGui::Checkbox("Inherit Speed", &emitter->inherit_speed)) need_save = true;

		ImGui::Separator();
		if (emitter->initial_lifetime.DrawEditor("Lifetime")) need_save = true;

		ImGui::Separator();
		if (emitter->initial_pos.DrawEditor()) need_save = true;

		ImGui::Separator();
		if (emitter->initial_speed.DrawEditor("Starting Speed")) need_save = true;

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

		if (emitter->external_acc.DrawEditor()) need_save = true;

		ImGui::Separator();
		if (emitter->boundary.DrawEditor()) need_save = true;

		ImGui::Separator();
		if (emitter->collider.DrawEditor()) need_save = true;

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

		if (emitter->meshMD5)
		{
			if (ImGui::Button(eastl::string("Resource Mesh").c_str()))
				RE_RES->PushSelected(emitter->meshMD5, true);
		}
		else if (!emitter->primCmp)
			ImGui::TextWrapped("Select mesh resource or select primitive");
		else if (emitter->primCmp->DrawPrimPropierties()) need_save = true;


		ImGui::Separator();

		static bool clearMesh = false, setUpPrimitive = false;
		if (ImGui::BeginMenu("Primitive"))
		{
			if (ImGui::MenuItem("Point"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompPoint();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("Cube"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompCube();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("Dodecahedron"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompDodecahedron();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("Tetrahedron"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompTetrahedron();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("Octohedron"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompOctohedron();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("Icosahedron"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompIcosahedron();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("Plane"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompPlane();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("Sphere"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompSphere();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("Cylinder"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompCylinder();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("HemiSphere"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompHemiSphere();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("Torus"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompTorus();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("Trefoil Knot"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompTrefoiKnot();
				setUpPrimitive = clearMesh = true;
			}
			if (ImGui::MenuItem("Rock"))
			{
				if (emitter->primCmp)
				{
					emitter->primCmp->UnUseResources();
					DEL(emitter->primCmp)
				}
				emitter->primCmp = new RE_CompRock();
				setUpPrimitive = clearMesh = true;
			}

			ImGui::EndMenu();
		}

		if (clearMesh)
		{
			need_save = true;

			if (emitter->meshMD5)
			{
				RE_RES->UnUse(emitter->meshMD5);
				emitter->meshMD5 = nullptr;
			}

			if (setUpPrimitive)
			{
				RE_SCENE->primitives->SetUpComponentPrimitive(emitter->primCmp);
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
					if (emitter->meshMD5) RE_RES->UnUse(emitter->meshMD5);
					emitter->meshMD5 = m->GetMD5();
					if (emitter->meshMD5) RE_RES->Use(emitter->meshMD5);

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

		if (emitter->light.DrawEditor(simulation))
			need_save = true;

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

		if (ImGui::DragFloat3("Scale", emitter->scale.ptr(), 0.1f, -10000.f, 10000.f, "%.2f"))
		{
			if (!emitter->scale.IsFinite()) emitter->scale.Set(0.5f, 0.5f, 0.5f);
			need_save = true;
		}

		int pDir = static_cast<int>(emitter->orientation);
		if (ImGui::Combo("Particle Direction", &pDir, "Normal\0Billboard\0Custom\0"))
		{
			emitter->orientation = static_cast<RE_ParticleEmitter::ParticleDir>(pDir);
			need_save = true;
		}

		if (emitter->orientation == RE_ParticleEmitter::ParticleDir::Custom)
		{
			ImGui::DragFloat3("Custom Direction", emitter->direction.ptr(), 0.1f, -1.f, 1.f, "%.2f");
			need_save = true;
		}

		if (emitter->color.DrawEditor()) need_save = true;
		if (emitter->opacity.DrawEditor()) need_save = true;

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

		if (emitter->opacity.type != RE_PR_Opacity::Type::VALUE
			&& emitter->opacity.type != RE_PR_Opacity::Type::NONE
			&& emitter->opacity.useCurve)
		{
			if (emitter->opacity.curve.DrawEditor("Opacity"))
				need_save = true;
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

		if (emitter->color.type != RE_PR_Color::Type::SINGLE && emitter->color.useCurve)
		{
			if (emitter->color.curve.DrawEditor("Color"))
				need_save = true;
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

	if (!close) return;

	if (!need_save)
	{
		CloseEditor();
		return;
	}

	if (emiter_md5)
	{
		auto emitter = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emiter_md5));
		bool has_emissor = emitter->HasEmissor();
		bool has_render = emitter->HasRenderer();
		RE_EDITOR->popupWindow->PopUpSaveParticles((!has_emissor || !has_render), true, has_emissor, has_render, true);
	}
	else RE_EDITOR->popupWindow->PopUpSaveParticles(true, false, new_emitter->HasEmissor(), new_emitter->HasRenderer(), true);
}
