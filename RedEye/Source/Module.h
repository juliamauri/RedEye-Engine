#ifndef __MODULE_H__
#define __MODULE_H__

#include "RapidJson\include\document.h"
#include "Globals.h"
#include "EventListener.h"

class Module : public EventListener
{
private:

	const char* name;
	bool enabled;

public:

	Module(const char* module_name, bool start_enabled = true) : enabled(start_enabled), name(module_name) {}
	virtual ~Module(){}

	bool IsActive() const { return enabled; }
	const char* GetName() const { return name; }

	virtual bool Init(rapidjson::Value::ConstMemberIterator config_module) { return true; } //SETTING OWN VALUES
	virtual bool Start(/* CONFIG */) { return true; } //ACCESS OTHER MODULES
	
	virtual update_status PreUpdate() { return UPDATE_CONTINUE; }
	virtual update_status Update() { return UPDATE_CONTINUE; }
	virtual update_status PostUpdate() { return UPDATE_CONTINUE; }

	virtual bool CleanUp() { return true; }

	virtual void Load(){}
	virtual void Save() const {}

	//virtual void RecieveEvent(const Event* e) override {}
};

#endif //__MODULE_H__