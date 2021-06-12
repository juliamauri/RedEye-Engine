#ifndef __MODULE_H__
#define __MODULE_H__

#include "EventListener.h"

class Module : public EventListener
{
protected:

	const char* name;

public:

	Module(const char* module_name) : EventListener(), name(module_name) {}
	virtual ~Module() {}

	const char* GetName() const { return name; }

	virtual bool Init() { return true; } // SETTING OWN VALUES
	virtual bool Start() { return true; } // ACCESS OTHER MODULES
	virtual void CleanUp() {} // clear heap allocations
	
	virtual void PreUpdate() {}
	virtual void Update() {}
	virtual void PostUpdate() {}

	virtual void DrawEditor() {}
	virtual void RecieveEvent(const Event& e) override {}

	virtual void Load() {}
	virtual void Save() const {}
};

#endif //__MODULE_H__