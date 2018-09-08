#ifndef __APP_H__
#define __APP_H__

#include "EventListener.h"
//#include "Cvar.h"
#include <list>
#include <map>

class Module;
class ModuleWindow;
class ModuleInput;
class ModuleEditor;
class ModuleRenderer3D;

class FileSystem;
class TimeManager;

class Application : public EventListener
{
public:
	Application(int argc, char* argv[]);
	~Application();

	bool Init();
	int Update();
	bool CleanUp();

	void DrawEditor();
	void Log(const char* text);
	void RequestBrowser(const char* link) const;
	void RecieveEvent(const Event* e) override;

private:

	void PrepareUpdate();
	void FinishUpdate();

public:

	ModuleWindow* window = nullptr;
	ModuleInput* input = nullptr;
	ModuleEditor* editor = nullptr;
	ModuleRenderer3D* renderer3d = nullptr;

	FileSystem* fs = nullptr;
	TimeManager* time = nullptr;

private:

	std::list<Module*> modules;
	bool want_to_quit = false;

	//std::map<const char*, DoubleCvar> config_vars;

	int argc;
	char* argv[];
};

extern Application* App;

#endif // !__APP_H__