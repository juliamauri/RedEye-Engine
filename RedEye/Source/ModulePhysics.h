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

private:

	ParticleManager particles;
};

#endif // !__MODULEPHYSICS__