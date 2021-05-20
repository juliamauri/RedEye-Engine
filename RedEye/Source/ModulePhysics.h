#ifndef __MODULEPHYSICS__
#define __MODULEPHYSICS__

#include "Module.h"
#include "ParticleManager.h"

class ModulePhysics : public Module
{
public:
	ModulePhysics();
	~ModulePhysics();

	bool Init() override;
	bool Start() override;
	void Update() override;
	void CleanUp() override;

	RE_ParticleEmitter* AddEmitter();
	void RemoveEmitter(RE_ParticleEmitter* emitter);

	unsigned int GetParticleCount(unsigned int emitter_id) const;
	eastl::list<RE_Particle*>* GetParticles(unsigned int emitter_id) const;

private:

	ParticleManager particles;
};

#endif // !__MODULEPHYSICS__