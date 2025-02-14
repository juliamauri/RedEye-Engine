#include "ModulePhysics.h"

#include "RE_Profiler.h"
#include "Application.h"
#include "ModuleScene.h"
#include "RE_FileSystem.h"
#include "RE_PrimitiveManager.h"
#include "RE_Time.h"
#include "RE_CompCamera.h"

#include <GL/glew.h>

void ModulePhysics::Update()
{
	RE_PROFILE(RE_ProfiledFunc::Update, RE_ProfiledClass::ModulePhysics);
	float global_dt = RE_Time::DeltaTime();

	switch (mode) {
	case ModulePhysics::UpdateMode::FIXED_UPDATE:
	{
		dt_offset += global_dt;

		float final_dt = 0.f;
		while (dt_offset >= fixed_dt)
		{
			dt_offset -= fixed_dt;
			final_dt += fixed_dt;
		}

		if (final_dt > 0.f)
		{
			update_count++;
			particles.Update(final_dt);
		}

		break;
	}
	case ModulePhysics::UpdateMode::FIXED_TIME_STEP:
	{
		dt_offset += global_dt;

		while (dt_offset >= fixed_dt)
		{
			dt_offset -= fixed_dt;
			update_count++;
			particles.Update(fixed_dt);
		}

		break;
	}
	default:
	{
		update_count++;
		particles.Update(global_dt);
		break;
	}
	}

	time_counter += global_dt;
	if (time_counter >= 1.f)
	{
		time_counter--;
		updates_per_s = update_count;
		update_count = 0.f;
	}
}

void ModulePhysics::CleanUp()
{
	particles.Clear();
}

void ModulePhysics::OnPlay(const bool was_paused)
{
	for (auto sim : particles.simulations)
	{
		if (was_paused)
		{
			if (sim->state == RE_ParticleEmitter::PlaybackState::PAUSE)
				sim->state = RE_ParticleEmitter::PlaybackState::RESTART;
			else
				sim->state = RE_ParticleEmitter::PlaybackState::STOPING;
		}
		else if (sim->start_on_play)
			sim->state = RE_ParticleEmitter::PlaybackState::RESTART;
		else 
			sim->state = RE_ParticleEmitter::PlaybackState::STOPING;
	}
}

void ModulePhysics::OnPause()
{
	for (auto sim : particles.simulations)
		if (sim->state == RE_ParticleEmitter::PlaybackState::PLAY)
			sim->state = RE_ParticleEmitter::PlaybackState::PAUSE;
}

void ModulePhysics::OnStop()
{
	for (auto sim : particles.simulations)
		sim->state = RE_ParticleEmitter::PlaybackState::STOPING;
}

void ModulePhysics::DrawParticleEmitterSimulation(unsigned int index, math::float3 go_positon, math::float3 go_up) const
{
	particles.DrawSimulation(index, go_positon, go_up);
}

void ModulePhysics::DebugDrawParticleEmitterSimulation(const RE_ParticleEmitter* const sim) const
{
	particles.DebugDrawSimulation(sim, particles.GetInterval());
}

void ModulePhysics::CallParticleEmitterLightShaderUniforms(unsigned int index, math::float3 go_position, unsigned int shader, const char* array_unif_name, unsigned int& count, unsigned int maxLights, bool sharedLight) const
{
	particles.CallLightShaderUniforms(index, go_position, shader, array_unif_name, count, maxLights, sharedLight);
}

void ModulePhysics::DrawDebug(RE_CompCamera* current_camera) const
{
	if (!particles.simulations.empty())
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(current_camera->GetProjectionPtr());
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf((current_camera->GetView()).ptr());
		particles.DrawDebug();
	}
}

void ModulePhysics::DrawEditor()
{
	ImGui::Text("Updates/s: %.1f", updates_per_s);

	int type = static_cast<int>(mode);
	if (ImGui::Combo("Update Mode", &type, "Engine Par\0Fixed Update\0Fixed Time Step\0"))
		mode = static_cast<UpdateMode>(type);

	if (type)
	{
		float period = 1.f / fixed_dt;
		if (ImGui::DragFloat("Dt", &period, 1.f, 1.f, 480.f, "%.0f"))
			fixed_dt = 1.f / period;
	}

	particles.DrawEditor();
}

void ModulePhysics::Load()
{
	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::ModuleWindow);
	RE_LOG_SECONDARY("Loading Physics propieties from config:");
	RE_Json* node = RE_FS->ConfigNode("Physics");

	mode = static_cast<UpdateMode>(node->PullInt("UpdateMode", static_cast<int>(UpdateMode::FIXED_UPDATE)));

	DEL(node)
}

void ModulePhysics::Save() const
{
	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::ModulePhysics);
	RE_Json* node = RE_FS->ConfigNode("Physics");

	node->Push("UpdateMode", static_cast<int>(mode));

	DEL(node)
}

RE_ParticleEmitter* ModulePhysics::AddEmitter(RE_ParticleEmitter* to_add)
{
	particles.Allocate(to_add);
	return to_add;
}

void ModulePhysics::RemoveEmitter(RE_ParticleEmitter* emitter)
{
	particles.Deallocate(emitter->id);
}

unsigned int ModulePhysics::GetParticleCount(unsigned int emitter_id) const
{
	for (const auto sim : particles.simulations)
		if (sim->id == emitter_id)
			return sim->particle_count;

	return 0u;
}

bool ModulePhysics::GetParticles(unsigned int emitter_id, eastl::vector<RE_Particle>& out) const
{
	for (const auto& sim : particles.simulations)
	{
		if (sim->id == emitter_id)
		{
			out = sim->particle_pool;
			return true;
		}
	}

	return false;
}
