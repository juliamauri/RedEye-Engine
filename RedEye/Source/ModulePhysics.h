#ifndef __MODULEPHYSICS__
#define __MODULEPHYSICS__

#include "Module.h"
#include "ParticleManager.h"

class ModulePhysics : public Module
{
public:
	ModulePhysics();
	~ModulePhysics();

	// Module
	bool Init();
	bool Start();
	//void PreUpdate() override;
	void Update();
	void CleanUp();
	//void RecieveEvent(const Event& e) override;
	//void DrawEditor() override;

	RE_ParticleEmitter* AddEmitter();
	void RemoveEmitter(RE_ParticleEmitter* emitter);

	unsigned int GetParticleCount(unsigned int emitter_id) const;

private:

	ParticleManager particles;
};

#endif // !__MODULEPHYSICS__