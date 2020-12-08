#ifndef __MODULE_H__
#define __MODULE_H__

#include "Globals.h"
#include "EventListener.h"

class Module : public EventListener
{
protected:

	const char* name;
	bool enabled;

public:

	Module(const char* module_name, bool start_enabled = true) : EventListener(), enabled(start_enabled), name(module_name) {}
	virtual ~Module(){}

	bool IsActive() const { return enabled; }
	const char* GetName() const { return name; }

	virtual bool Init() { return true; } // SETTING OWN VALUES
	virtual bool Start() { return true; } // ACCESS OTHER MODULES
	virtual void CleanUp() {} // clear heap allocations
	
	virtual void PreUpdate() {}
	virtual void Update() {}
	virtual void PostUpdate() {}

	virtual void DrawEditor() {}
	virtual void RecieveEvent(const Event& e) override {}

	virtual void OnPlay() {}
	virtual void OnPause() {}
	virtual void OnStop() {}

	virtual void Load() {}
	virtual void Save() const {}
};

#endif //__MODULE_H__