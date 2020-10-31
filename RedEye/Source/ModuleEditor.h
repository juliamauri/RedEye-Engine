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
class SceneEditorWindow;
class SceneGameWindow;

struct SoftwareInfo;
class RE_GameObject;
class RE_Component;
class RE_CompCamera;
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
	void DrawEditor() override;

	void RecieveEvent(const Event& e) override;

	void DrawDebug(RE_CompCamera* cam) const;
	void DrawHeriarchy();

	RE_GameObject* GetSelected() const;
	void SetSelected(RE_GameObject* go, bool force_focus = false);
	void DuplicateSelectedObject();

	void LogToEditorConsole();
	bool AddSoftwareUsed(const char * name, const char * version, const char * website);
	void Draw();
	void HandleSDLEvent(SDL_Event* e);

	void PopUpFocus(bool focus);
	PopUpWindow* popupWindow = nullptr;

	const char* GetAssetsPanelPath()const;

	void SelectUndefinedFile(eastl::string* toSelect)const;

	void OpenTextEditor(const char* filePath, eastl::string* filePathStr, const char* shadertTemplate = nullptr, bool* open = nullptr);

	void GetSceneWindowSize(unsigned int* widht, unsigned int* height);

	void CreatePrefab(RE_GameObject* go, const char* name, bool identityRoot);

	void PushCommand(RE_Command* cmd);
	void ClearCommands();

	void DrawGameObjectItems(RE_GameObject* parent = nullptr);

private:
	void UpdateCamera();


public:

	bool debug_drawing = true;

private:

	bool show_all = true;
	bool show_demo = false;
	bool popUpFocus = false;

	eastl::list<EditorWindow*> windows, tools;

	RE_CommandManager editorCommands;

	// Windows
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

	SceneEditorWindow* sceneEditorWindow = nullptr;
	SceneGameWindow* sceneGameWindow = nullptr;
	// Tools
	RandomTest* rng = nullptr;

	// Camera Controls
	bool select_on_mc = true;
	bool focus_on_select = false;
	float cam_speed = 5.0f;
	float cam_sensitivity = 0.01f;

	// Selected GO
	RE_GameObject* selected = nullptr;

	// Debug Drawing
	AABBDebugDrawing aabb_drawing = AABBDebugDrawing::ALL_AND_SELECTED;
	bool draw_quad_tree = true;
	bool draw_cameras = true;

	float all_aabb_color[3];
	float sel_aabb_color[3];
	float quad_tree_color[3];
	float frustum_color[3];

	// Grid
	ComponentsPool* editorCompsPool = nullptr;
	RE_Component* grid = nullptr;
	RE_GameObject* grid_go = nullptr;
	float grid_size[2];

	//crtd security
	bool isDuplicated = false;
};

#endif // !__MODULEEDITOR__
