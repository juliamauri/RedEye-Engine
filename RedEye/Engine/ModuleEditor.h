#ifndef __MODULEEDITOR__
#define __MODULEEDITOR__

#include "EventListener.h"
#include "RE_DataTypes.h"
#include <EASTL/list.h>

class ModuleEditor : public EventListener
{
public:
	ModuleEditor();
	~ModuleEditor() final;

	// Module
	bool Init();
	bool Start();
	void PreUpdate();
	void Update();
	void CleanUp();
	void RecieveEvent(const Event& e) override final;
	void DrawEditor();

	void Load();
	void Save() const;

	// Draws
	void Draw() const;
	void DrawDebug(class RE_CompCamera* current_camera) const;
	void DrawHeriarchy();

	// UI
	void DrawGameObjectItems(const GO_UID parent = 0);

	// Selection
	GO_UID GetSelected() const;
	void SetSelected(const GO_UID go, bool force_focus = false);
	void DuplicateSelectedObject();

	// Editor Windows
	void ReportSoftawe(const char* name, const char* version, const char* website) const;
	void HandleSDLEvent(union SDL_Event* e);
	void PopUpFocus(bool focus);
	const char* GetAssetsPanelPath() const;
	void SelectUndefinedFile(eastl::string* toSelect) const;
	void OpenTextEditor(const char* filePath, eastl::string* filePathStr, const char* shadertTemplate = nullptr, bool* open = nullptr);
	void GetSceneWindowSize(unsigned int* widht, unsigned int* height);
	void StartEditingParticleEmitter(class RE_ParticleEmitter* sim, const char* md5);
	const RE_ParticleEmitter* GetCurrentEditingParticleEmitter()const;
	void SaveEmitter(bool close = false, const char* emitter_name = nullptr, const char* emissor_base = nullptr, const char* renderer_base = nullptr);
	void CloseParticleEditor();
	bool IsParticleEditorActive() const;
	bool EditorSceneNeedsRender() const;
	bool GameSceneNeedsRender() const;

	// Commands
	void PushCommand(class RE_Command* cmd);
	void ClearCommands();

	class SceneEditorWindow* GetSceneEditor() { return sceneEditorWindow; }

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
	eastl::list<class EditorWindow*> windows;

	// General Windows
	class ConsoleWindow* console = nullptr;
	class AssetsWindow* assets = nullptr;
	class WwiseWindow* wwise = nullptr;
	class ConfigWindow* config = nullptr;
	class HierarchyWindow* hierarchy = nullptr;
	class PropertiesWindow* properties = nullptr;
	class PlayPauseWindow* play_pause = nullptr;
	class AboutWindow* about = nullptr;
	class MaterialEditorWindow* materialeditor = nullptr;
	class ShaderEditorWindow* shadereditor = nullptr;
	class SkyBoxEditorWindow* skyboxeditor = nullptr;
	class TextEditorManagerWindow* texteditormanager = nullptr;
	class WaterPlaneWindow* waterplaneWindow = nullptr;
	class ParticleEmitterEditorWindow* particleEmitterWindow = nullptr;

	// Scene views
	SceneEditorWindow* sceneEditorWindow = nullptr;
	class GameWindow* sceneGameWindow = nullptr;

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
	GO_UID selected = 0;

	// Grid
	class RE_CompGrid* grid = nullptr;
	float grid_size[2];

	// Debug Drawing
	enum class AABBDebugDrawing : int
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
