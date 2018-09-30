#ifndef __APP_H__
#define __APP_H__

#include "EventListener.h"
//#include "Cvar.h"
#include <string>
#include <list>
#include <map>

class Module;
class ModuleInput;
class ModuleWindow;
class ModuleScene;
class ModuleEditor;
class ModuleRenderer3D;

class FileSystem;
class TimeManager;
class SystemInfo;
class RE_Math;
struct OutputLogHolder;

class Application : public EventListener
{
public:
	Application(int argc, char* argv[]);
	~Application();

	bool Init();
	int Update();
	bool CleanUp();

	bool Load();
	bool Save();

	void DrawEditor();
	void Log(const char* text, const char* file);
	void ReportSoftware(const char * name, const char * version = nullptr, const char * website = nullptr);
	void RecieveEvent(const Event* e) override;

	const char* GetName() const;
	const char* GetOrganization() const;

private:

	void PrepareUpdate();
	void FinishUpdate();

public:

	ModuleInput* input = nullptr;
	ModuleWindow* window = nullptr;
	ModuleScene* scene = nullptr;
	ModuleEditor* editor = nullptr;
	ModuleRenderer3D* renderer3d = nullptr;

	FileSystem* fs = nullptr;
	TimeManager* time = nullptr;
	SystemInfo* sys_info = nullptr;
	RE_Math* math = nullptr;
	OutputLogHolder* log = nullptr;

private:

	std::list<Module*> modules;
	bool want_to_load_def = false;
	bool want_to_load = false;
	bool want_to_save = false;
	bool want_to_quit = false;

	std::string app_name = "RedEye Engine";
	std::string organization = "RedEye";

	int argc;
	char* argv[];
};

extern Application* App;

#endif // !__APP_H__