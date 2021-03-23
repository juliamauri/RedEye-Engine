#ifndef __APP_H__
#define __APP_H__

#include "EventListener.h"
#include "RE_ConsoleLog.h"

class RE_Time;
class RE_Math;
class RE_Hardware;

class Module;
class ModuleInput;
class ModuleWindow;
class ModuleScene;
class ModulePhysics;
class ModuleEditor;
class ModuleRenderer3D;
class ModuleAudio;

class RE_FileSystem;
class RE_ResourceManager;

class Application : public EventListener
{
public:

	Application();
	~Application();

	bool Init(int argc, char* argv[]);
	void MainLoop();
	void CleanUp();

	void RecieveEvent(const Event& e) override;

	const char* GetName() { return "RedEye Engine"; }
	const char* GetOrganization() { return "RedEye"; }

private:

	void LoadConfig();
	void SaveConfig();

public:

	int argc = 0;
	char** argv = nullptr;

	// Utility
	RE_ConsoleLog log;
	RE_Time* time = nullptr;
	RE_Math* math = nullptr;
	RE_Hardware* hardware = nullptr;

	// Modules
	ModuleInput* input = nullptr;
	ModuleWindow* window = nullptr;
	ModuleScene* scene = nullptr;
	ModulePhysics* physics = nullptr;
	ModuleEditor* editor = nullptr;
	ModuleRenderer3D* renderer = nullptr;
	ModuleAudio* audio = nullptr;

	// Files & Resources
	RE_FileSystem* fs = nullptr;
	RE_ResourceManager* res = nullptr;

private:

	enum AppFlags : char
	{
		EMPLY_FLAGS = 0,
		LOAD_CONFIG = 0x1,	// 0001
		SAVE_CONFIG = 0x2,	// 0010
		WANT_TO_QUIT = 0x4,	// 0100
		SAVE_ON_EXIT = 0x8,	// 1000
	};

	char flags = EMPLY_FLAGS;
};

extern Application* App;

#define RE_LOGGER App->log
#define RE_TIME App->time
#define RE_MATH App->math
#define RE_HARDWARE App->hardware
#define RE_FS App->fs
#define RE_RES App->res

#define RE_INPUT App->input
#define RE_WINDOW App->window
#define RE_SCENE App->scene
#define RE_EDITOR App->editor
#define RE_RENDER App->renderer
#define RE_AUDIO App->audio

/*
RE_INPUT->
RE_WINDOW->
RE_SCENE->
RE_EDITOR->
RE_RENDER->
RE_AUDIO->

RE_LOGGER.
RE_TIME->
RE_RNG->
RE_HARDWARE->
RE_FS->
RE_RES->
*/

#endif // !__APP_H__