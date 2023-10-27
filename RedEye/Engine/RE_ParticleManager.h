#ifndef __RE_PARTICLE_MANAGER_H__
#define __RE_PARTICLE_MANAGER_H__

#include "RE_DataTypes.h"
#include "RE_ParticleEmitter.h"
#include "RE_Camera.h"

#include <EASTL/map.h>

class RE_Json;

namespace RE_ParticleManager
{
	enum class BoundingMode : int { GENERAL, PER_PARTICLE };

	namespace
	{
		BoundingMode bounding_mode;
		uint particle_count = 0;
		eastl::map<P_UID, RE_ParticleEmitter&> simulations;
	}

	void UpdateAllSimulations(const float dt);
	bool UpdateEmitter(P_UID id, const math::vec& global_pos);
	void ClearSimulations();

	P_UID Allocate(RE_ParticleEmitter& emitter, P_UID id = 0);
	bool Deallocate(P_UID id);

	void OnPlay(bool was_paused);
	void OnPause();
	void OnStop();

	RE_ParticleEmitter* GetEmitter(P_UID id);
	bool EmitterHasBlend(P_UID id);
	bool EmitterHasLight(P_UID id);
	bool SetEmitterState(P_UID id, RE_ParticleEmitter::PlaybackState state);

	void DrawEditor();
	void DrawDebugAll();

	uint GetTotalParticleCount();
	uint GetParticleCount(P_UID id);
	bool GetParticles(P_UID id, eastl::vector<RE_Particle>& out);
	BoundingMode GetBoundingMode();

	void DrawSimulation(
		P_UID id,
		const RE_Camera& camera,
		math::float3 go_position = math::float3::zero,
		math::float3 go_up = math::float3::unitY);
	
	void CallLightShaderUniforms(
		P_UID id,
		math::float3 go_position,
		uint shader,
		const char* array_unif_name,
		uint& count,
		uint maxLights,
		bool sharedLight);

	void Load(RE_Json* node);
	void Save(RE_Json* node);
};

#endif //!__RE_PARTICLE_MANAGER_H__