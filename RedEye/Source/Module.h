#ifndef __MODULE_H__
#define __MODULE_H__

#include "Globals.h"

class Module
{
private:

	const char* name;
	bool enabled;

public:

	Module(const char* module_name, bool start_enabled = true) : enabled(start_enabled)
	{
		name = module_name;
	}
	virtual ~Module(){}

	bool IsActive() { return enabled; }

	virtual bool Init(/* CONFIG */) { return true; } //SETTING VALUES
	virtual bool Start(/* CONFIG */) { return true; } //ACCESS OTHER MODULES
	
	virtual update_status PreUpdate() { return UPDATE_CONTINUE; }
	virtual update_status Update() { return UPDATE_CONTINUE; }
	virtual update_status PostUpdate() { return UPDATE_CONTINUE; }

	virtual bool CleanUp() { return true; }

	virtual void Load(){}
	virtual void Save(){}
};

#endif //__MODULE_H__