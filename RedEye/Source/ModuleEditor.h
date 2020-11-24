#ifndef __MODULEEDITOR__
#define __MODULEEDITOR__

#include "Module.h"

#include "RE_CommandManager.h"

#include "ImGui\imgui.h"
#include <EASTL/list.h>

class EditorWindow;
class ConsoleWindow;
class AssetsWindow;
class WwiseWindow;
class DemoWindow;
class ConfigWindow;
class HeriarchyWindow;
class PropertiesWindow;
class AboutWindow;
class RandomTest;
class PlayPauseWindow;
class PopUpWindow;
class MaterialEditorWindow;
class ShaderEditorWindow;
class SkyBoxEditorWindow;
class TextEditorManagerWindow;
class WaterPlaneResourceWindow;
class SceneEditorWindow;
class SceneGameWindow;

class TransformDebugWindow;

struct SoftwareInfo;
class RE_GameObject;
class RE_Component;
class RE_CompCamera;
class RE_CompGrid;
class ComponentsPool;

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
	ModuleEditor(const char* name = "Editor", bool start_enabled = true);
	~ModuleEditor();

	bool Init(JSONNode* node) override;
	bool Start() override;
	update_status PreUpdate() override;
	update_status Update() override;
	bool CleanUp() override;
	void RecieveEvent(const Event& e) override;

	// Draws
	void Draw() const;
	void DrawEditor() override;
	void DrawDebug(RE_CompCamera* current_camera) const;
	void DrawHeriarchy();

	// UI
	void DrawGameObjectItems(const UID parent = 0);
	void CreatePrefab(const UID go, const char* name, bool identityRoot);

	// Selection
	UID GetSelected() const;
	void SetSelected(const UID go, bool force_focus = false);
	void DuplicateSelectedObject();

	// Logs
	void LogToEditorConsole();
	bool AddSoftwareUsed(const char * name, const char * version, const char * website);

	// Editor Windows
	void HandleSDLEvent(SDL_Event* e);
	void PopUpFocus(bool focus);
	const char* GetAssetsPanelPath()const;
	void SelectUndefinedFile(eastl::string* toSelect)const;
	void OpenTextEditor(const char* filePath, eastl::string* filePathStr, const char* shadertTemplate = nullptr, bool* open = nullptr);
	void GetSceneWindowSize(unsigned int* widht, unsigned int* height);

	// Commands
	void PushCommand(RE_Command* cmd);
	void ClearCommands();

	SceneEditorWindow* GetSceneEditor() { return sceneEditorWindow; }

private:

	void UpdateCamera();

public:

	bool debug_drawing = true;

	PopUpWindow* popupWindow = nullptr;

private:

	// Flags
	bool show_all = true;
	bool show_demo = false;
	bool popUpFocus = false;

	// Command Tracker
	RE_CommandManager editorCommands;

	// Windows & Tools
	eastl::list<EditorWindow*> windows, tools;

	// General Windows
	ConsoleWindow* console = nullptr;
	AssetsWindow* assets = nullptr;
	WwiseWindow* wwise = nullptr;
	ConfigWindow* config = nullptr;
	HeriarchyWindow* heriarchy = nullptr;
	PropertiesWindow* properties = nullptr;
	PlayPauseWindow* play_pause = nullptr;
	AboutWindow* about = nullptr;
	MaterialEditorWindow* materialeditor = nullptr;
	ShaderEditorWindow* shadereditor = nullptr;
	SkyBoxEditorWindow* skyboxeditor = nullptr;
	TextEditorManagerWindow* texteditormanager = nullptr;
	WaterPlaneResourceWindow* waterplaneResourceWindow = nullptr;

	// Scene views
	SceneEditorWindow* sceneEditorWindow = nullptr;
	SceneGameWindow* sceneGameWindow = nullptr;

	// Tools
	RandomTest* rng = nullptr;

	//Debug info
	TransformDebugWindow* transDebInfo = nullptr;

	// Camera Controls
	bool select_on_mc = true;
	bool focus_on_select = false;
	float cam_speed = 5.0f;
	float cam_sensitivity = 0.01f;

	// Selected GO
	UID selected = 0;

	// Grid
	RE_CompGrid* grid = nullptr;
	float grid_size[2];

	// Debug Drawing
	AABBDebugDrawing aabb_drawing = AABBDebugDrawing::ALL_AND_SELECTED;
	bool draw_quad_tree = true;
	bool draw_cameras = true;

	float all_aabb_color[3],  sel_aabb_color[3], quad_tree_color[3], frustum_color[3];

	//crtd security
	bool isDuplicated = false;
};

#endif // !__MODULEEDITOR__
