#ifndef __MODULE_H__
#define __MODULE_H__

#include "Globals.h"
#include "EventListener.h"

class JSONNode;

class Module : public EventListener
{
private:

	const char* name;
	bool enabled;

public:

	Module(const char* module_name, bool start_enabled = true) : EventListener(), enabled(start_enabled), name(module_name) {}
	virtual ~Module(){}

	bool IsActive() const { return enabled; }
	const char* GetName() const { return name; }

	EventListener* AsEventListener() { return this; }
	const EventListener* AsEventListener() const { return this; }

	virtual bool Init(JSONNode* node) { return true; } //SETTING OWN VALUES
	virtual bool Start() { return true; } //ACCESS OTHER MODULES
	
	virtual update_status PreUpdate() { return UPDATE_CONTINUE; }
	virtual update_status Update() { return UPDATE_CONTINUE; }
	virtual update_status PostUpdate() { return UPDATE_CONTINUE; }

	virtual void DrawEditor() {}

	virtual bool CleanUp() { return true; }

	virtual bool Load(JSONNode* node) { return true; }
	virtual bool Save(JSONNode* node) const { return true; }

	virtual void RecieveEvent(const Event* e) override {}
};

#endif //__MODULE_H__