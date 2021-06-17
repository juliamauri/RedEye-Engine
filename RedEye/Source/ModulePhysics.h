#ifndef __MODULEPHYSICS__
#define __MODULEPHYSICS__

#include "Module.h"
#include "ParticleManager.h"

class RE_CompCamera;

class ModulePhysics : public Module
{
public:
	ModulePhysics();
	~ModulePhysics();

	void Update() override;
	void CleanUp() override;

	void DrawDebug(RE_CompCamera* current_camera) const;
	void DrawEditor() override;

	RE_ParticleEmitter* AddEmitter();
	void RemoveEmitter(RE_ParticleEmitter* emitter);

	unsigned int GetParticleCount(unsigned int emitter_id) const;
	eastl::list<RE_Particle*>* GetParticles(unsigned int emitter_id) const;

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