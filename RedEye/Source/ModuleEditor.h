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
class PlayPauseWindow;
class SelectFile;
class PrefabsPanel;
class PopUpWindow;
class SkyBoxWindow;

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

	void DrawEditor() override;

	void LogToEditorConsole();
	bool AddSoftwareUsed(const char * name, const char * version, const char * website);
	void Draw();
	void HandleSDLEvent(SDL_Event* e);

	//Select file
	SelectFile* GetSelectWindow()const;

	void PopUpFocus(bool focus);
	PopUpWindow* popupWindow = nullptr;
	SkyBoxWindow* skyBoxWindow = nullptr;

private:

	void UpdateCamera();

private:

	bool show_all = true;
	bool show_demo = false;
	bool popUpFocus = false;

	// Camera
	float cam_speed = 5.0f;
	float cam_sensitivity = 0.01f;

	// Windows
	std::list<EditorWindow*> windows, tools;
	ConsoleWindow* console = nullptr;
	ConfigWindow* config = nullptr;
	HeriarchyWindow* heriarchy = nullptr;
	PropertiesWindow* properties = nullptr;
	PlayPauseWindow* play_pause = nullptr;
	PrefabsPanel* prefabsPanel = nullptr;

	AboutWindow* about = nullptr;
	SelectFile* select_file = nullptr;

	// Tools
	RandomTest* rng = nullptr;
	TexturesWindow* textures = nullptr;

	//crtd security
	bool isDuplicated = false;
};

#endif // !__MODULEEDITOR__