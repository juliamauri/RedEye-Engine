#ifndef __MODULESCENE_H__
#define __MODULESCENE_H__

#include "Module.h"
#include "Event.h"

class ModuleScene : public Module
{
public:
	ModuleScene(const char* name, bool start_enabled = true);
	~ModuleScene();

	//bool Init(JSONNode* config_module) override;
	bool CleanUp() override;

	void FileDrop(const char* file);

	void RecieveEvent(const Event* e) override;

private:

	//GameObject root;
};


#endif // !__MODULESCENE_H__