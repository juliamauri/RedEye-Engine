#ifndef __APP_H__
#define __APP_H__

#include "Event.h"
#include "RE_ConsoleLog.h"

class Application : public EventListener
{
public:

	Application();
	~Application();

	bool Init(int argc, char* argv[]);
	void MainLoop();
	void CleanUp();
	void Quit();
	void RecieveEvent(const Event& e) override;

private:

	void LoadConfig();
	void SaveConfig();

public:

	// Utility
	RE_ConsoleLog log;

	class RE_Time* time = nullptr;
	class RE_Math* math = nullptr;
	class RE_Hardware* hardware = nullptr;

	// Modules
	class ModuleInput* input = nullptr;
	class ModuleWindow* window = nullptr;
	class ModuleScene* scene = nullptr;
	class ModulePhysics* physics = nullptr;
	class ModuleEditor* editor = nullptr;
	class ModuleRenderer3D* renderer = nullptr;
	class ModuleAudio* audio = nullptr;

	// Files & Resources
	class RE_FileSystem* fs = nullptr;
	class RE_ResourceManager* res = nullptr;

private:

	int argc = 0;
	char** argv = nullptr;

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
#define RE_PHYSICS App->physics
#define RE_EDITOR App->editor
#define RE_RENDER App->renderer
#define RE_AUDIO App->audio

#endif // !__APP_H__