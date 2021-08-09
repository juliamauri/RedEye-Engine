#ifndef __SCENE_EDITOR_WINDOW__
#define __SCENE_EDITOR_WINDOW__

#include <ImGuiWidgets/ImGuizmo/ImGuizmo.h>

class SceneEditorWindow :public EditorWindow
{
public:
	SceneEditorWindow(const char* name = "Editor Scene", bool start_active = true) : EditorWindow(name, start_active) {}
	~SceneEditorWindow() {}

	unsigned int GetSceneWidht()const { return (width == 0) ? 500 : width; }
	unsigned int GetSceneHeight()const { return (heigth == 0) ? 500 : heigth; }

	bool isSelected()const { return isWindowSelected; }
	bool NeedRender()const { return need_render; };

	void Recalc() { recalc = true; }

	ImGuizmo::OPERATION GetOperation() const { return operation; }
	ImGuizmo::MODE GetMode() const { return mode; }

	void SetOperation(ImGuizmo::OPERATION o) { operation = o; }
	void SetMode(ImGuizmo::MODE m) { mode = m; }

	void UpdateViewPort();

private:

	void Draw(bool secondary = false) override;

	math::float4 viewport = math::float4::zero;
	int width = 0;
	int heigth = 0;

	ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE mode = ImGuizmo::MODE::LOCAL;

	bool isWindowSelected = false;
	bool recalc = false;
	bool need_render = true;
};

#endif //!__SCENE_EDITOR_WINDOW__