#ifndef __MODULESCENE_H__
#define __MODULESCENE_H__

#include "Module.h"
#include "Event.h"

class RE_GameObject;
class RE_CompUnregisteredMesh;

class ModuleScene : public Module
{
public:
	ModuleScene(const char* name, bool start_enabled = true);
	~ModuleScene();

	bool Start() override;
	update_status Update() override;
	bool CleanUp() override;

	void FileDrop(const char* file);
	void RecieveEvent(const Event* e) override;

	void DrawScene();
	void DrawFocusedProperties();


private:
	// Root
	RE_GameObject* root = nullptr;

	//shaders
	unsigned int modelloading;
};


#endif // !__MODULESCENE_H__