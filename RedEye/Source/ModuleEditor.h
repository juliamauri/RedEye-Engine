#ifndef __MODULEEDITOR__
#define __MODULEEDITOR__

#include "Globals.h"
#include "Module.h"
#include <EASTL/list.h>

class SceneEditorWindow;

class ModuleEditor : public Module
{
public:
	ModuleEditor();
	~ModuleEditor();

	// Module
	bool Init() override;
	bool Start() override;
	void PreUpdate() override;
	void Update() override;
	void CleanUp() override;
	void RecieveEvent(const Event& e) override;
	void DrawEditor() override;

	// Draws
	void Draw() const;
	void DrawDebug(class RE_CompCamera* current_camera) const;
	void DrawHeriarchy();

	// UI
	void DrawGameObjectItems(const UID parent = 0);

	// Selection
	UID GetSelected() const;
	void SetSelected(const UID go, bool force_focus = false);
	void DuplicateSelectedObject();

	// Editor Windows
	void ReportSoftawe(const char* name, const char* version, const char* website) const;
	void HandleSDLEvent(union SDL_Event* e);
	void PopUpFocus(bool focus);
	const char* GetAssetsPanelPath() const;
	void SelectUndefinedFile(eastl::string* toSelect) const;
	void OpenTextEditor(const char* filePath, eastl::string* filePathStr, const char* shadertTemplate = nullptr, bool* open = nullptr);
	void GetSceneWindowSize(unsigned int* widht, unsigned int* height);
	void StartEditingParticleEmiter(class RE_ParticleEmitter* sim, UID fromComponent);
	bool IsParticleEditorActive() const;
	UID GetEditingParticleEmittorComponent() const;

	// Commands
	void PushCommand(class RE_Command* cmd);
	void ClearCommands();

	SceneEditorWindow* GetSceneEditor() { return sceneEditorWindow; }

private:

	void UpdateCamera();

public:

	class RE_CommandManager* commands = nullptr;
	class RE_ThumbnailManager* thumbnails = nullptr;

	bool debug_drawing = true;

	class PopUpWindow* popupWindow = nullptr;

private:

	// Flags
	bool show_all = true;
	bool show_demo = false;
	bool popUpFocus = false;

	// Windows & Tools
	eastl::list<class EditorWindow*> windows, tools;

	// General Windows
	class ConsoleWindow* console = nullptr;
	class AssetsWindow* assets = nullptr;
	class WwiseWindow* wwise = nullptr;
	class ConfigWindow* config = nullptr;
	class HeriarchyWindow* heriarchy = nullptr;
	class PropertiesWindow* properties = nullptr;
	class PlayPauseWindow* play_pause = nullptr;
	class AboutWindow* about = nullptr;
	class MaterialEditorWindow* materialeditor = nullptr;
	class ShaderEditorWindow* shadereditor = nullptr;
	class SkyBoxEditorWindow* skyboxeditor = nullptr;
	class TextEditorManagerWindow* texteditormanager = nullptr;
	class WaterPlaneResourceWindow* waterplaneResourceWindow = nullptr;
	class ParticleEmiiterEditorWindow* particleEmitterWindow = nullptr;

	// Scene views
	SceneEditorWindow* sceneEditorWindow = nullptr;
	class SceneGameWindow* sceneGameWindow = nullptr;

	// Tools
	class RandomTest* rng = nullptr;

	//Debug info
	class TransformDebugWindow* transDebInfo = nullptr;
	class RendererDebugWindow* rendDebInfo = nullptr;

	// Camera Controls
	bool select_on_mc = true;
	bool focus_on_select = false;
	float cam_speed = 25.0f;
	float cam_sensitivity = 0.01f;

	// Selected GO
	UID selected = 0;

	// Grid
	class RE_CompGrid* grid = nullptr;
	float grid_size[2];

	// Debug Drawing
	enum AABBDebugDrawing : int
	{
		NONE = 0,
		SELECTED_ONLY,
		ALL,
		ALL_AND_SELECTED,
	} aabb_drawing = AABBDebugDrawing::ALL_AND_SELECTED;
	bool draw_quad_tree = true;
	bool draw_cameras = true;

	float all_aabb_color[3],  sel_aabb_color[3], quad_tree_color[3], frustum_color[3];

	//crtd security
	bool isDuplicated = false;
};

#endif // !__MODULEEDITOR__
