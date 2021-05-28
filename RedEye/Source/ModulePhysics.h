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

private:

	void SpawnParticles(RE_ParticleEmitter* emitter, eastl::list<RE_Particle*>* container, const float dt) const;
	void ImpulseCollision(RE_ParticleEmitter* emitter, RE_Particle& p1, RE_Particle& p2) const;
	void ImpulseCollisionTS(RE_ParticleEmitter* emitter, RE_Particle& p1, RE_Particle& p2, const float dt) const;
	void ApplyParticleSpeed(RE_ParticleEmitter* emitter, RE_Particle& p1, const float dt) const;
	
private:

	ParticleManager particles;

	enum CollisionResolution : int
	{
		SIMPLE,
		Thomas_Smid
	} method = SIMPLE;

	// Debug drawing
	float debug_color[4] = { 0.1f, 0.8f, 0.1f, 1.f };
};

#endif // !__MODULEPHYSICS__