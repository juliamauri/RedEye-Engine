#include "RE_ParticleManager.h"

#include "RE_Memory.h"
#include "RE_Math.h"
#include "RE_Random.h"
#include "RE_Time.h"
#include "RE_Profiler.h"
#include "Application.h"
#include "RE_ResourceManager.h"
#include "ModuleRenderer3D.h"
#include "RE_Json.h"

#include "RE_InternalResources.h"
#include "RE_ShaderImporter.h"
#include "RE_Shader.h"
#include "RE_GLCache.h"
#include "RE_Mesh.h"
#include "RE_Particle.h"

#include "RE_CompTransform.h"
#include "RE_CompPrimitive.h"
#include "RE_CompLight.h"

void RE_ParticleManager::UpdateAllSimulations(const float dt)
{
	RE_PROFILE(RE_ProfiledFunc::Update, RE_ProfiledClass::ParticleManager)

	particle_count = 0;
	for (auto& sim : simulations) particle_count += sim.second.Update(dt);
}

bool RE_ParticleManager::UpdateEmitter(P_UID id, const math::vec& global_pos)
{
	auto sim = simulations.find(id);
	if (sim == simulations.end()) return false;

	auto& emitter = sim->second;
	emitter.parent_speed = (global_pos - emitter.parent_pos) / RE_Time::DeltaTime();
	emitter.parent_pos = global_pos;

	return emitter.state <= RE_ParticleEmitter::PlaybackState::PLAY;
}

void RE_ParticleManager::ClearSimulations()
{
	simulations.clear();
}

P_UID RE_ParticleManager::Allocate(RE_ParticleEmitter& emitter, P_UID id)
{
	if (id == 0) id = RE_Random::RandomUID();
	simulations.insert({ id, emitter });
	return id;
}

bool RE_ParticleManager::Deallocate(P_UID id) { return simulations.erase(id) > 0; }

void RE_ParticleManager::OnPlay(bool was_paused)
{
	for (auto& sim : simulations)
	{
		if (was_paused)
		{
			if (sim.second.state == RE_ParticleEmitter::PlaybackState::PAUSE)
				sim.second.state = RE_ParticleEmitter::PlaybackState::RESTART;
			else
				sim.second.state = RE_ParticleEmitter::PlaybackState::STOPING;
		}
		else if (sim.second.start_on_play)
			sim.second.state = RE_ParticleEmitter::PlaybackState::RESTART;
		else
			sim.second.state = RE_ParticleEmitter::PlaybackState::STOPING;
	}
}

void RE_ParticleManager::OnPause()
{
	for (auto& sim : simulations)
		if (sim.second.state == RE_ParticleEmitter::PlaybackState::PLAY)
			sim.second.state = RE_ParticleEmitter::PlaybackState::PAUSE;
}

void RE_ParticleManager::OnStop()
{
	for (auto& sim : simulations)
		sim.second.state = RE_ParticleEmitter::PlaybackState::STOPING;
}

RE_ParticleEmitter* RE_ParticleManager::GetEmitter(P_UID id)
{
	auto sim = simulations.find(id);
	if (sim == simulations.end()) return nullptr;
	return &sim->second;
}

bool RE_ParticleManager::EmitterHasBlend(P_UID id)
{
	auto sim = simulations.find(id);
	if (sim == simulations.end()) return false;
	return static_cast<bool>(sim->second.opacity.type);
}

bool RE_ParticleManager::EmitterHasLight(P_UID id)
{
	auto sim = simulations.find(id);
	if (sim == simulations.end()) return false;
	return sim->second.light.HasLight();
}

bool RE_ParticleManager::SetEmitterState(P_UID id, RE_ParticleEmitter::PlaybackState state)
{
	auto sim = simulations.find(id);
	if (sim == simulations.end()) return false;
	sim->second.state = state;
	return true;
}

void RE_ParticleManager::DrawEditor()
{
	ImGui::Text("Total Emitters: %i", simulations.size());
	ImGui::Text("Total Particles: %i", particle_count);

	int tmp = static_cast<int>(bounding_mode);
	if (ImGui::Combo("AABB Enclosing", &tmp, "General\0Per Particle\0"))
		bounding_mode = static_cast<BoundingMode>(tmp);
}

void RE_ParticleManager::DrawDebugAll()
{
	RE_PROFILE(RE_ProfiledFunc::DrawDebug, RE_ProfiledClass::ModulePhysics)

	for (const auto& sim : simulations)
		RE_RENDER->DebugDrawParticleEmitter(sim.second);
}

uint RE_ParticleManager::GetTotalParticleCount() { return particle_count; }

uint RE_ParticleManager::GetParticleCount(P_UID id)
{
	auto sim = simulations.find(id);
	if (sim == simulations.end()) return 0;
	return sim->second.particle_count;
}

bool RE_ParticleManager::GetParticles(P_UID id, eastl::vector<RE_Particle>& out)
{
	auto sim = simulations.find(id);
	if (sim == simulations.end()) return false;
	out = sim->second.particle_pool;
	return true;
}
RE_ParticleManager::BoundingMode RE_ParticleManager::GetBoundingMode() { return bounding_mode; }

void RE_ParticleManager::DrawSimulation(P_UID id, const RE_Camera& camera, math::float3 go_position, math::float3 go_up)
{
	RE_PROFILE(RE_ProfiledFunc::DrawParticles, RE_ProfiledClass::ParticleManager)

	auto sim = simulations.find(id);
	if (sim == simulations.end() || !sim->second.active_rendering) return;

	// Get Shader and uniforms
	const RE_Shader* pS = static_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetParticleShader()));
	const unsigned int shader = pS->GetID();
	RE_GLCache::ChangeShader(shader);

	RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);

	auto& simulation = sim->second;
	for (const auto& p : simulation.particle_pool)
	{
		// Calculate Particle Transform
		const math::float3 partcleGlobalpos = simulation.local_space ? go_position + p.position : p.position;
		math::float3 front, right, up;
		switch (simulation.orientation)
		{
		case RE_ParticleEmitter::ParticleDir::FromPS:
		{
			front = (go_position - partcleGlobalpos).Normalized();
			right = front.Cross(go_up).Normalized();
			if (right.x < 0.0f) right *= -1.0f;
			up = right.Cross(front).Normalized();
			break;
		}
		case RE_ParticleEmitter::ParticleDir::Billboard:
		{
			auto& frustum = camera.GetFrustum();
			front = frustum.Pos() - partcleGlobalpos;
			front.Normalize();
			right = front.Cross(frustum.Up()).Normalized();
			if (right.x < 0.0f) right *= -1.0f;
			up = right.Cross(front).Normalized();
			break;
		}
		case RE_ParticleEmitter::ParticleDir::Custom:
		{
			front = simulation.direction.Normalized();
			right = front.Cross(camera.GetFrustum().Up()).Normalized();
			if (right.x < 0.0f) right *= -1.0f;
			up = right.Cross(front).Normalized();
			break;
		}
		}

		// Rotate and upload Transform
		math::float4x4 rotm;
		rotm.SetRotatePart(math::float3x3(right, up, front));
		math::vec roteuler = rotm.ToEulerXYZ();
		pS->UploadModel(math::float4x4::FromTRS(partcleGlobalpos, math::Quat::identity * math::Quat::FromEulerXYZ(roteuler.x, roteuler.y, roteuler.z), simulation.scale).Transposed().ptr());

		float weight = 1.f;

		// Lightmode
		if (ModuleRenderer3D::GetLightMode() == RenderSettings::LightMode::DEFERRED)
		{
			bool cNormal = !simulation.meshMD5 && !simulation.primCmp;
			RE_ShaderImporter::setFloat(shader, "customNormal", static_cast<float>(cNormal));
			if (cNormal) RE_ShaderImporter::setFloat(shader, "normal", front);
			RE_ShaderImporter::setFloat(shader, "specular", 2.5f);
			RE_ShaderImporter::setFloat(shader, "shininess", 16.0f);
			RE_ShaderImporter::setFloat(shader, "opacity", 1.0f);
		}
		else
		{
			//Opacity
			switch (simulation.opacity.type)
			{
			case RE_PR_Opacity::Type::OVERLIFETIME: weight = p.lifetime / simulation.initial_lifetime.GetMax(); break;
			case RE_PR_Opacity::Type::OVERDISTANCE: weight = math::SqrtFast(p.position.LengthSq()) / math::SqrtFast(simulation.max_dist_sq); break;
			case RE_PR_Opacity::Type::OVERSPEED: weight = math::SqrtFast(p.velocity.LengthSq()) / math::SqrtFast(simulation.max_speed_sq); break;
			default: break;
			}

			RE_ShaderImporter::setFloat(shader, "opacity", simulation.opacity.GetValue(weight));
		}

		// Color
		switch (simulation.color.type)
		{
		case RE_PR_Color::Type::OVERLIFETIME: weight = p.lifetime / simulation.initial_lifetime.GetMax(); break;
		case RE_PR_Color::Type::OVERDISTANCE: weight = math::SqrtFast(p.position.LengthSq()) / math::SqrtFast(simulation.max_dist_sq); break;
		case RE_PR_Color::Type::OVERSPEED: weight = math::SqrtFast(p.velocity.LengthSq()) / math::SqrtFast(simulation.max_speed_sq); break;
		default: break;
		}

		RE_ShaderImporter::setFloat(shader, "cdiffuse", simulation.color.GetValue(weight));

		// Draw Call
		if (simulation.meshMD5) dynamic_cast<RE_Mesh*>(RE_RES->At(simulation.meshMD5))->DrawMesh(shader);
		else
		{
			RE_CompPrimitive* primitive_component = simulation.primCmp;
			if (primitive_component) primitive_component->SimpleDraw();
		}
	}
}

void RE_ParticleManager::CallLightShaderUniforms(
	P_UID id,
	math::float3 go_position,
	uint shader,
	const char* array_unif_name,
	uint& count,
	uint maxLights,
	bool sharedLight)
{
	RE_PROFILE(RE_ProfiledFunc::DrawParticlesLight, RE_ProfiledClass::ParticleManager)
	
	auto sim = simulations.find(id);
	if (sim == simulations.end()) return;

	eastl::string array_name(array_unif_name);
	array_name += "[";
	eastl::string unif_name;

	auto& simulation = sim->second;

	if (!sharedLight)
	{
		eastl::string uniform_name("pInfo.tclq");
		RE_ShaderImporter::setFloat(
			RE_ShaderImporter::getLocation(shader, (uniform_name).c_str()),
			static_cast<float>(RE_CompLight::Type::POINT),
			simulation.light.constant,
			simulation.light.linear,
			simulation.light.quadratic);

		for (const auto& p : simulation.particle_pool)
		{
			if (count == maxLights) return;
			unif_name = array_name + eastl::to_string(count++) + "].";

			const math::float3 p_global_pos = simulation.local_space ? go_position + p.position : p.position;
			switch (simulation.light.type) {
			case RE_PR_Light::Type::UNIQUE:
			{
				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "diffuseSpecular").c_str()),
					simulation.light.color.x, simulation.light.color.y, simulation.light.color.z, simulation.light.specular);

				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "positionIntensity").c_str()),
					p_global_pos.x, p_global_pos.y, p_global_pos.z, simulation.light.intensity);

				break;
			}
			case RE_PR_Light::Type::PER_PARTICLE:
			{
				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "diffuseSpecular").c_str()),
					p.lightColor.x, p.lightColor.y, p.lightColor.z, p.specular);

				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "positionIntensity").c_str()),
					p_global_pos.x, p_global_pos.y, p_global_pos.z, p.intensity);

				break;
			}
			default: break;
			}
		}
	}
	else
	{
		const math::vec color = simulation.light.GetColor();
		const float intensity = simulation.light.GetIntensity();

		for (const auto& p : simulation.particle_pool)
		{
			if (count == maxLights) return;

			unif_name = array_name + eastl::to_string(count++) + "].";

			switch (simulation.light.type)
			{
			case RE_PR_Light::Type::UNIQUE:
			{
				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "directionIntensity").c_str()),
					0.f, 0.f, 0.f, intensity);

				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "diffuseSpecular").c_str()),
					color.x, color.y, color.z, simulation.light.GetSpecular());
				break;
			}
			case RE_PR_Light::Type::PER_PARTICLE:
			{
				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "directionIntensity").c_str()),
					0.f, 0.f, 0.f, p.intensity);

				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "diffuseSpecular").c_str()),
					p.lightColor.x, p.lightColor.y, p.lightColor.z, p.specular);
				break;
			}
			default: break;
			}

			const math::float3 partcleGlobalpos = go_position + p.position;
			RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(
				shader,
				(unif_name + "positionType").c_str()),
				partcleGlobalpos.x,
				partcleGlobalpos.y,
				partcleGlobalpos.z,
				static_cast<float>(RE_CompLight::Type::POINT));

			const math::vec quadratic = simulation.light.GetQuadraticValues();
			RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "clq").c_str()), quadratic.x, quadratic.y, quadratic.z, 0.f);
		}
	}
}

void RE_ParticleManager::Load(RE_Json* node)
{
	node->PullInt("BoundingMode", static_cast<int>(bounding_mode));
	DEL(node)
}

void RE_ParticleManager::Save(RE_Json* node)
{
	node->Push("BoundingMode", static_cast<int>(bounding_mode));
	DEL(node)
}
