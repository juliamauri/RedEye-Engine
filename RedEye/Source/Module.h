#pragma once

#ifndef __MODULE_H__
#define __MODULE_H__

class Application;

class Module
{
private:
	bool enabled;

public:
	Application * App;

	Module(Application* parent, bool start_enabled = true) : App(parent)
	{}
	virtual ~Module()
	{}

	virtual bool Init() { return true; }

	virtual void Awake(){}
	virtual bool Start() { return true; }
	
	virtual bool PreUpdate() { return true; }
	virtual bool Update() { return true; }
	virtual bool PostUpdate() { return true; }

	virtual bool CleanUp() { return true; }

	virtual void Load(){}
	virtual void Save(){}
};

#endif //__MODULE_H__