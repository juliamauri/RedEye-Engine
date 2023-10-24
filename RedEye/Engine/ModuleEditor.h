#ifndef __MODULEEDITOR__
#define __MODULEEDITOR__

#include "EventListener.h"
#include "RE_DataTypes.h"
#include <EASTL/list.h>

class RE_Camera;

class ModuleEditor : public EventListener
{
public:

	enum class Flag : int
	{
		// Config
		SHOW_EDITOR = 1 << 0,
		SHOW_IMGUI_DEMO = 1 << 1,

		// State
		POPUP_IS_FOCUSED = 1 << 2
	};

	class PopUpWindow* popupWindow = nullptr;

private:

	// Flags
	ushort flags = 0;

	// Selected GO
	GO_UID selected;

	// Base Windows
	eastl::list<class EditorWindow*> windows;
	class ConsoleWindow* console = nullptr;
	class ConfigWindow* config = nullptr;
	class HierarchyWindow* hierarchy = nullptr;
	class PropertiesWindow* properties = nullptr;
	class PlayPauseWindow* play_pause = nullptr;
	class AssetsWindow* assets = nullptr;
	class WwiseWindow* wwise = nullptr;
	class AboutWindow* about = nullptr;

	// Resource Editors
	class MaterialEditorWindow* materialeditor = nullptr;
	class ShaderEditorWindow* shadereditor = nullptr;
	class SkyBoxEditorWindow* skyboxeditor = nullptr;
	class TextEditorManagerWindow* texteditormanager = nullptr;
	class WaterPlaneWindow* waterplaneWindow = nullptr;

	// Scene views
	eastl::list<class RenderedWindow*> rendered_windows;
	class SceneEditorWindow* sceneEditorWindow = nullptr;
	class GameWindow* sceneGameWindow = nullptr;
	class ParticleEmitterEditorWindow* particleEmitterWindow = nullptr;

	// Tools
	class RandomTest* rng = nullptr;

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
	inline void AddFlag(Flag flag) { flags |= static_cast<int>(flag); }
	inline void RemoveFlag(Flag flag) { flags -= static_cast<int>(flag); }
	inline const bool HasFlag(Flag flag) const { return flags & static_cast<int>(flag); }
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
	
	/*/ Particle Editor Window
	void StartEditingParticleEmitter(RE_ParticleEmitter* sim, const char* md5);
	const RE_ParticleEmitter* GetCurrentEditingParticleEmitter() const;
	void SaveEmitter(bool close = false, const char* emitter_name = nullptr, const char* emissor_base = nullptr, const char* renderer_base = nullptr);
	void CloseParticleEditor();
	bool IsParticleEditorActive() const;
	RE_Camera* GetParticlesCamera() const;*/

	/*void ModuleEditor::StartEditingParticleEmitter(RE_ParticleEmitter* sim, const char* md5)
		particleEmitterWindow->StartEditing(sim, md5);
	
	const RE_ParticleEmitter* ModuleEditor::GetCurrentEditingParticleEmitter() const
		return particleEmitterWindow->GetEdittingParticleEmitter();

	void ModuleEditor::SaveEmitter(bool close, const char* emitter_name, const char* emissor_base, const char* renderer_base)
		particleEmitterWindow->SaveEmitter(close, emitter_name, emissor_base, renderer_base);

	void ModuleEditor::CloseParticleEditor() { particleEmitterWindow->NextOrClose(); }
	bool ModuleEditor::IsParticleEditorActive() const { return particleEmitterWindow->IsActive(); }
	RE_Camera* ModuleEditor::GetParticlesCamera() const { return &(particleEmitterWindow->GetCamera()); }*/

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
