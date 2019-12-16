#ifndef __APP_H__
#define __APP_H__

#include "EventListener.h"
#include "Optick/include/optick.h"
#include <string>
#include <list>
#include <map>

class Module;
class ModuleInput;
class ModuleWindow;
class ModuleScene;
class ModuleEditor;
class ModuleRenderer3D;

class RE_FileSystem;
class TimeManager;
class SystemInfo;
class RE_Math;
class OutputLogHolder;
class RE_CameraManager;
class RE_TextureImporter;
class RE_ShaderImporter;
class RE_PrimitiveManager;
class RE_ModelImporter;
class RE_ResourceManager;
class RE_HandleErrors;
class RE_InternalResources;
class RE_GLCache;
class RE_FBOManager;
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
	int Update();
	bool CleanUp();

	bool Load();
	bool Save();

	void DrawEditor();
	void Log(const int category, const char* text, const char* file);
	void ReportSoftware(const char * name, const char * version = nullptr, const char * website = nullptr);
	void RecieveEvent(const Event& e) override;

	const char* GetName() const;
	const char* GetOrganization() const;

	GameState GetState() const;

private:

	void PrepareUpdate();
	void FinishUpdate();

public:
	ModuleInput* input = nullptr;
	ModuleWindow* window = nullptr;
	ModuleScene* scene = nullptr;
	ModuleEditor* editor = nullptr;
	ModuleRenderer3D* renderer3d = nullptr;

	RE_FileSystem* fs = nullptr;
	TimeManager* time = nullptr;
	SystemInfo* sys_info = nullptr;
	RE_Math* math = nullptr;
	OutputLogHolder* log = nullptr;

	RE_CameraManager* cams = nullptr;
	RE_TextureImporter* textures = nullptr;
	RE_ShaderImporter* shaders = nullptr;
	RE_PrimitiveManager* primitives = nullptr;
	RE_ModelImporter* modelImporter = nullptr;

	RE_ResourceManager* resources = nullptr;
	RE_InternalResources* internalResources = nullptr;

	RE_HandleErrors* handlerrors = nullptr;

	RE_GLCache* glcache = nullptr;
	RE_FBOManager* fbomanager = nullptr;

	RE_ThumbnailManager* thumbnail = nullptr;

private:

	std::list<Module*> modules;
	bool want_to_load_def = false;
	bool want_to_load = false;
	bool want_to_save = false;
	bool want_to_quit = false;
	bool ticking = false;

	std::string app_name = "RedEye Engine";
	std::string organization = "RedEye";

	GameState state = GS_STOP;
};

extern Application* App;

#endif // !__APP_H__