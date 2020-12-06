#ifndef __APP_H__
#define __APP_H__

#include "EventListener.h"

#include "RE_Math.h"

#include "RE_TimeManager.h"
#include "RE_CameraManager.h"
#include "RE_PrimitiveManager.h"

#include "RE_TextureImporter.h"
#include "RE_ModelImporter.h"
#include "RE_ShaderImporter.h"

#include "Optick/include/optick.h"
#include <EASTL/string.h>
#include <EASTL/list.h>

class RE_FileSystem;
class RE_Hardware;

class Module;
class ModuleInput;
class ModuleWindow;
class ModuleScene;
class ModuleEditor;
class ModuleRenderer3D;
class ModuleAudio;

class RE_ResourceManager;
class RE_ThumbnailManager;

enum GameState : char
{
	GS_PLAY,
	GS_PAUSE,
	GS_STOP
};

class Application : public EventListener
{
public:

	Application();
	~Application();

	bool Init(int argc, char* argv[]);
	void MainLoop();
	void CleanUp();

	void DrawEditor();
	void RecieveEvent(const Event& e) override;

	static Application* Ptr();
	static const char* GetName();
	static const char* GetOrganization();
	static GameState GetState();

private:

	bool Load();
	bool Save();

public:

	// Utility
	static RE_FileSystem* fs;
	static RE_Math math;
	static RE_Hardware* hardware;

	// Modules
	static ModuleInput* input;
	static ModuleWindow* window;
	static ModuleScene* scene;
	static ModuleEditor* editor;
	static ModuleRenderer3D* renderer3d;
	static ModuleAudio* audio;

	// Managers
	static RE_TimeManager time;
	static RE_CameraManager cams;
	static RE_PrimitiveManager primitives;
	static RE_ThumbnailManager* thumbnail;
	static RE_ResourceManager* resources;

	// Importers
	static RE_TextureImporter textures;
	static RE_ModelImporter modelImporter;
	static RE_ShaderImporter shaders;

private:

	bool want_to_load = false;
	bool want_to_save = false;
	bool want_to_quit = false;
	bool ticking = false;

	static GameState state;
	static Application* instance;
	static eastl::string app_name;
	static eastl::string organization;
	static eastl::list<Module*> modules;
};

typedef Application App;

#endif // !__APP_H__