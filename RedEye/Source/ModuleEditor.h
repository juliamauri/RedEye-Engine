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
class PopUpWindow;

struct SoftwareInfo;
class RE_GameObject;
class RE_CompCamera;

union SDL_Event;

enum AABBDebugDrawing : int
{
	NONE = 0,
	SELECTED_ONLY,
	ALL,
	ALL_AND_SELECTED,
};

class ModuleEditor : public Module
{
public:
	ModuleEditor(const char* name, bool start_enabled = true);
	~ModuleEditor();

	bool Init(JSONNode* node) override;
	bool Start() override;
	update_status PreUpdate() override;
	update_status Update() override;
	bool CleanUp() override;
	void DrawEditor() override;

	void DrawDebug(bool resetLight) const;
	void DrawHeriarchy();

	RE_GameObject* GetSelected() const;
	void SetSelected(RE_GameObject* go, bool force_focus = false);
	void DuplicateSelectedObject();

	void LogToEditorConsole();
	bool AddSoftwareUsed(const char * name, const char * version, const char * website);
	void Draw();
	void HandleSDLEvent(SDL_Event* e);

	//Select file
	SelectFile* GetSelectWindow()const;

	void PopUpFocus(bool focus);
	PopUpWindow* popupWindow = nullptr;

private:

	void UpdateCamera();

private:

	bool show_all = true;
	bool show_demo = false;
	bool popUpFocus = false;

	std::list<EditorWindow*> windows, tools;

	// Windows
	ConsoleWindow* console = nullptr;
	ConfigWindow* config = nullptr;
	HeriarchyWindow* heriarchy = nullptr;
	PropertiesWindow* properties = nullptr;
	PlayPauseWindow* play_pause = nullptr;
	AboutWindow* about = nullptr;
	SelectFile* select_file = nullptr;

	// Tools
	RandomTest* rng = nullptr;
	TexturesWindow* textures = nullptr;

	// Camera Controls
	bool select_on_mc = true;
	bool focus_on_select = false;
	float cam_speed = 5.0f;
	float cam_sensitivity = 0.01f;

	// Selected GO
	RE_GameObject* selected = nullptr;

	// Debug Drawing
	bool debug_drawing = true;
	AABBDebugDrawing aabb_drawing = AABBDebugDrawing::ALL_AND_SELECTED;
	bool draw_grid = true;
	bool draw_quad_tree = true;
	bool draw_cameras = true;

	float all_aabb_color[3];
	float sel_aabb_color[3];
	float quad_tree_color[3];
	float frustum_color[3];

	//crtd security
	bool isDuplicated = false;
};

#endif // !__MODULEEDITOR__
