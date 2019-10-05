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
class EditorSettingsWindow;
class PlayPauseWindow;
class SelectFile;
class PrefabsPanel;

struct SoftwareInfo;
class RE_CompCamera;

#define CAM_SENSITIVITY 0.01
#define CAM_SPEED 5.0

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

	void LogToEditorConsole();
	bool AddSoftwareUsed(const char * name, const char * version, const char * website);
	void Draw();
	void HandleSDLEvent(SDL_Event* e);

	// Camera
	RE_CompCamera* GetCamera() const;

	//Select file
	SelectFile* GetSelectWindow();

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
	EditorSettingsWindow* editor_settings = nullptr;
	PlayPauseWindow* play_pause = nullptr;

	AboutWindow* about = nullptr;

	SelectFile* select_file = nullptr;

	PrefabsPanel* prefabsPanel = nullptr;

	// Tools
	RandomTest* rng = nullptr;
	TexturesWindow* textures = nullptr;

	//crtd security
	bool isDuplicated = false;

	std::list<EditorWindow*> windows, tools;
};

#endif // !__MODULEEDITOR__