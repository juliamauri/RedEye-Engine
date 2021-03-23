#ifndef __MODULEPHYSICS__
#define __MODULEPHYSICS__

#include "Module.h"

class ModulePhysics : public Module
{
public:
	ModulePhysics();
	~ModulePhysics();

	// Module
	bool Init() override;
	bool Start() override;
	void PreUpdate() override;
	void Update() override;
	void CleanUp() override;
	void RecieveEvent(const Event& e) override;
	void DrawEditor() override;


};

#endif // !__MODULEPHYSICS__