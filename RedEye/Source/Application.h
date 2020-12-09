#ifndef __APP_H__
#define __APP_H__

#include "EventListener.h"
#include "RE_CameraManager.h"
#include "RE_PrimitiveManager.h"
#include "Optick/include/optick.h"
#include <EASTL/string.h>
#include <EASTL/list.h>

class Config;
class Module;
class ModuleInput;
class ModuleWindow;
class ModuleScene;
class ModuleEditor;
class ModuleRenderer3D;
class ModuleAudio;

class RE_FileSystem;
class RE_ResourceManager;
class RE_ThumbnailManager;

class Application : public EventListener
{
public:

	Application();
	~Application();

	bool Init(int argc, char* argv[]);
	void MainLoop();
	void CleanUp();

	void RecieveEvent(const Event& e) override;

	static Application* Ptr();
	static const char* GetName();
	static const char* GetOrganization();
	static void DrawModuleEditorConfig();

private:

	bool InitSDL();
	bool InitFileSystem(int argc, char* argv[]);
	bool InitModules();
	void InitInternalSystems();
	bool StartModules();

	void LoadConfig();
	void SaveConfig();

public:

	// Utility
	static RE_FileSystem* fs;

	// Modules
	static ModuleInput* input;
	static ModuleWindow* window;
	static ModuleScene* scene;
	static ModuleEditor* editor;
	static ModuleRenderer3D* renderer3d;
	static ModuleAudio* audio;
	static Config* config;

	// Managers
	static RE_CameraManager cams;
	static RE_PrimitiveManager primitives;
	static RE_ThumbnailManager* thumbnail;
	static RE_ResourceManager* resources;

private:

	bool load_config = false;
	bool save_config = false;
	bool quit = false;
	bool save_on_exit = true;

	static Application* instance;
	static eastl::string app_name;
	static eastl::string organization;
	static eastl::list<Module*> modules;
};

typedef Application App;

#endif // !__APP_H__