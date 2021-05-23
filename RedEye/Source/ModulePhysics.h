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

	bool Init() override;
	bool Start() override;
	void Update() override;
	void CleanUp() override;

	void DrawDebug(RE_CompCamera* current_camera) const;
	void DrawEditor() override;

	RE_ParticleEmitter* AddEmitter();
	void RemoveEmitter(RE_ParticleEmitter* emitter);

	unsigned int GetParticleCount(unsigned int emitter_id) const;
	eastl::list<RE_Particle*>* GetParticles(unsigned int emitter_id) const;

private:

	ParticleManager particles;

	// Debug drawing
	float debug_color[4] = { 0.1f, 0.8f, 0.1f, 1.f };
	float circle_steps = 12.f;
};

#endif // !__MODULEPHYSICS__