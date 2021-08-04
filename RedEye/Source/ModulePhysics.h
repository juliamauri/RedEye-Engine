#ifndef __MODULEPHYSICS__
#define __MODULEPHYSICS__

#include "ParticleManager.h"

class RE_CompCamera;

class ModulePhysics
{
public:
	ModulePhysics();
	~ModulePhysics();

	void Update();
	void CleanUp();

	void OnPlay(const bool was_paused);
	void OnPause();
	void OnStop();

	void DrawParticleEmitterSimulation(unsigned int index, math::float3  go_positon, math::float3 go_up) const;
	void DebugDrawParticleEmitterSimulation(const RE_ParticleEmitter* const sim)const;
	void CallParticleEmitterLightShaderUniforms(unsigned int index, math::float3 go_position, unsigned int shader, const char* array_unif_name, unsigned int& count, unsigned int maxLights, bool sharedLight) const;

	void DrawDebug(RE_CompCamera* current_camera) const;
	void DrawEditor();

	RE_ParticleEmitter* AddEmitter(RE_ParticleEmitter* to_add);
	void RemoveEmitter(RE_ParticleEmitter* emitter);

	unsigned int GetParticleCount(unsigned int emitter_id) const;
	const eastl::vector<RE_Particle>& GetParticles(unsigned int emitter_id) const;

public:

	enum UpdateMode : int
	{
		ENGINE_PAR,
		FIXED_UPDATE,
		FIXED_TIME_STEP
	} mode = FIXED_UPDATE;

private:

	ParticleManager particles;

	float fixed_dt = 1.f / 30.f;
	float dt_offset = 0.f;

	float update_count = 0.f;
	float time_counter = 0.f;
	float updates_per_s = 0.f;
};

#endif // !__MODULEPHYSICS__