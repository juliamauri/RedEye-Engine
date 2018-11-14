#ifndef __MODULEEDITOR__
#define __MODULEEDITOR__

#include "Module.h"
#include "ImGui\imgui.h"
#include <list>

class EditorWindow;
class ConsoleWindow;
class DemoWindow;
class ConfigWindow;
class HeriarchyWindow;
class PropertiesWindow;
class AboutWindow;
class RandomTest;
class TexturesWindow;

struct SoftwareInfo;
class RE_CompCamera;

union SDL_Event;

class ModuleEditor : public Module
{
public:
	ModuleEditor(const char* name, bool start_enabled = true);
	~ModuleEditor();

	bool Init(JSONNode* node) override;
	update_status PreUpdate() override;
	update_status Update() override;
	bool CleanUp() override;

	//void DrawEditor() override;

	void LogToEditorConsole();
	bool AddSoftwareUsed(const char * name, const char * version, const char * website);
	void Draw();
	void HandleSDLEvent(SDL_Event* e);

	// Camera
	RE_CompCamera* GetCamera() const;

private:

	void UpdateCamera();

private:

	RE_CompCamera* camera = nullptr;

	bool show_all = true;
	bool show_demo = false;

	// Windows
	ConsoleWindow* console = nullptr;
	ConfigWindow* config = nullptr;
	HeriarchyWindow* heriarchy = nullptr;
	PropertiesWindow* properties = nullptr;
	AboutWindow* about = nullptr;

	RandomTest* rng = nullptr;
	TexturesWindow* textures = nullptr;

	std::list<EditorWindow*> windows, tools;
};

#endif // !__MODULEEDITOR__