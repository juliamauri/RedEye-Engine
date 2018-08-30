#ifndef __APP_H__
#define __APP_H__

#include "EventListener.h"
#include <list>

class Module;
class ModuleWindow;
class ModuleInput;

class FileSystem;

class Application : public EventListener
{
public:
	Application(int argc, char* argv[]);
	~Application();

	bool Init();
	int Update();
	bool CleanUp();

	void RecieveEvent(const Event* e) override;

private:

	void PrepareUpdate();
	void FinishUpdate();

public:

	ModuleWindow* window = nullptr;
	ModuleInput* input = nullptr;

	FileSystem* fs = nullptr;

private:

	std::list<Module*> modules;
	bool want_to_quit = false;

	int argc;
	char* argv[];
};

extern Application* App;

#endif // __APP_H__