#ifndef __MODULEEDITOR__
#define __MODULEEDITOR__

#include "EventListener.h"
#include "RE_DataTypes.h"

#include <EASTL/string.h>
#include <EASTL/list.h>

class ModuleEditor : public EventListener
{
public:

	class PopUpWindow* popupWindow = nullptr;

private:

	// Flags
	enum class Flag : ushort
	{
		// Config
		SHOW_EDITOR = 1 << 0,
		SHOW_IMGUI_DEMO = 1 << 1,

		// State
		POPUP_IS_FOCUSED = 1 << 2
	};
	ushort flags = 0;

	// Selected GO
	GO_UID selected = 0;

	// Base Windows
	class ConsoleWindow* console = nullptr;
	class ConfigWindow* config = nullptr;
	class HierarchyWindow* hierarchy = nullptr;
	class PropertiesWindow* properties = nullptr;
	class PlayPauseWindow* play_pause = nullptr;
	class AssetsWindow* assets = nullptr;
	class WwiseWindow* wwise = nullptr;
	eastl::list<class EditorWindow*> windows;

	class AboutWindow* about = nullptr;

	// Resource Editors
	class MaterialEditorWindow* materialeditor = nullptr;
	class ShaderEditorWindow* shadereditor = nullptr;
	class SkyBoxEditorWindow* skyboxeditor = nullptr;
	class TextEditorManagerWindow* texteditormanager = nullptr;
	class WaterPlaneWindow* waterplaneWindow = nullptr;

	// Rendered Windows
	class RenderedWindow* sceneGameWindow = nullptr;
	class SceneEditorWindow* sceneEditorWindow = nullptr;
	class ParticleEmitterEditorWindow* particleEmitterWindow = nullptr;
	eastl::list<RenderedWindow*> rendered_windows;

	// Debug Windows
	class TransformDebugWindow* transDebInfo = nullptr;
	class RendererDebugWindow* rendDebInfo = nullptr;

public:

	ModuleEditor();
	~ModuleEditor() final;

	// Module
	bool Init();
	bool Start();
	void PreUpdate();
	void Update();
	void CleanUp();
	void DrawEditor();

	// Config Serialization
	void Load();
	void Save() const;

	// Events
	void RecieveEvent(const Event& e) final;
	void HandleSDLEvent(union SDL_Event* e);

	// Draws
	void DrawEditorWindows() const;
	void RenderWindowFBOs() const;
	void DrawHeriarchy();

	// GO Selection
	GO_UID GetSelected() { return selected; }
	void SetSelected(GO_UID go, bool force_focus = false);
	void DuplicateSelectedObject();

	// Flags
	inline void AddFlag(Flag flag) { flags |= static_cast<ushort>(flag); }
	inline void RemoveFlag(Flag flag) { flags -= static_cast<ushort>(flag); }
	inline const bool HasFlag(Flag flag) const { return flags & static_cast<ushort>(flag); }
	void CheckboxFlag(const char* label, Flag flag);

#pragma region Editor Windows
	
	// Assets Window
	const char* GetAssetsPanelPath() const;
	void SelectUndefinedFile(eastl::string* toSelect) const;

	// Text Editor Window
	void OpenTextEditor(
		const char* filePath,
		eastl::string* filePathStr,
		const char* shadertTemplate = nullptr,
		bool* open = nullptr);

	// About Window
	void ReportSoftawe(const char* name, const char* version, const char* website) const;

	// Popup Window
	void SetPopUpFocus(bool focus);
	
	// Rendered Windows
	SceneEditorWindow* GetSceneEditor() const { return sceneEditorWindow; }
	ParticleEmitterEditorWindow* GetParticleEmitterEditorWindow() const { return particleEmitterWindow; }
	
#pragma endregion

private:

	// Init
	bool InitializeImGui();
	void ApplyRedeyeStyling();

	// Editor Update
	void DrawWindows();
	void CheckEditorInputs();
	void UpdateEditorCameras();

	// Main Menu Bar
	void DrawMainMenuBar();
	void DrawGameObjectItems(GO_UID parent = 0);
};

#endif // !__MODULEEDITOR__