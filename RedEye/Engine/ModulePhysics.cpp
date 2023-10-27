#include "ModulePhysics.h"

#include "RE_Profiler.h"
#include "RE_Time.h"
#include "Application.h"
#include "ModuleScene.h"
#include "RE_FileSystem.h"
#include "RE_PrimitiveManager.h"
#include "RE_ParticleManager.h"
#include "RE_CompCamera.h"

void ModulePhysics::Update()
{
	RE_PROFILE(RE_ProfiledFunc::Update, RE_ProfiledClass::ModulePhysics)
	float global_dt = RE_Time::DeltaTime();

	switch (mode)
	{
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
			RE_ParticleManager::UpdateAllSimulations(final_dt);
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
			RE_ParticleManager::UpdateAllSimulations(fixed_dt);
		}

		break;
	}
	default:
	{
		update_count++;
		RE_ParticleManager::UpdateAllSimulations(global_dt);
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

void ModulePhysics::CleanUp() { RE_ParticleManager::ClearSimulations(); }
void ModulePhysics::OnPlay(bool was_paused) { RE_ParticleManager::OnPlay(was_paused); }
void ModulePhysics::OnPause() { RE_ParticleManager::OnPause(); }
void ModulePhysics::OnStop() { RE_ParticleManager::OnStop(); }
void ModulePhysics::DrawDebug() { RE_ParticleManager::DrawDebugAll(); }

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

	RE_ParticleManager::DrawEditor();
}

void ModulePhysics::Load()
{
	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::ModuleWindow)
	RE_LOG_SECONDARY("Loading Physics propieties from config:");

	RE_Json* node = RE_FS->ConfigNode("Physics");
	mode = static_cast<UpdateMode>(node->PullInt("UpdateMode", static_cast<int>(mode)));
	RE_ParticleManager::Load(node->PullJObject("ParticleManager"));

	DEL(node)
}

void ModulePhysics::Save()
{
	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::ModulePhysics)

	RE_Json* node = RE_FS->ConfigNode("Physics");
	node->Push("UpdateMode", static_cast<int>(mode));
	RE_ParticleManager::Save(node->PushJObject("ParticleManager"));

	DEL(node)
}
