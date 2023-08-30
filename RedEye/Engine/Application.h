#ifndef __APP_H__
#define __APP_H__

#include "EventListener.h"
#include "Event.h"
#include "RE_ConsoleLog.h"

class Application : public EventListener
{
public:

	Application() = default;
	~Application() final;

	void AllocateModules();
	bool Init(int argc, char* argv[]);
	void MainLoop();
	void CleanUp();
	void Quit();
	void RecieveEvent(const Event& e) override;

private:

	bool InitModules();
	bool StartModules();

	void LoadConfig();
	void SaveConfig();

	enum class Flag : uchar
	{
		LOAD_CONFIG = 1 << 0,
		SAVE_CONFIG = 1 << 1,
		WANT_TO_QUIT = 1 << 2,
		SAVE_ON_EXIT = 1 << 3
	};

	inline void AddFlag(Flag flag);
	inline void RemoveFlag(Flag flag);
	inline const bool HasFlag(Flag flag) const;

public:

	// Files & Resources
	class RE_FileSystem* fs = nullptr;
	class RE_ResourceManager* res = nullptr;

	// Modules
	class ModuleInput* input = nullptr;
	class ModuleWindow* window = nullptr;
	class ModuleScene* scene = nullptr;
	class ModulePhysics* physics = nullptr;
	class ModuleEditor* editor = nullptr;
	class ModuleRenderer3D* renderer = nullptr;
	class ModuleAudio* audio = nullptr;

private:

	uchar flags = 0;

	int argc = 0;
	char** argv = nullptr;
};

extern Application* App;

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