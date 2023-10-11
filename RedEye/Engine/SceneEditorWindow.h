#ifndef __SCENE_EDITOR_WINDOW__
#define __SCENE_EDITOR_WINDOW__

#include "RenderedWindow.h"
#include <ImGuiWidgets/ImGuizmo/ImGuizmo.h>

class SceneEditorWindow :public RenderedWindow
{
public:
	SceneEditorWindow() : RenderedWindow("Editor Scene", true) {}
	~SceneEditorWindow() final = default;

	bool NeedRender() const { return need_render; };

	ImGuizmo::OPERATION GetOperation() const { return operation; }
	ImGuizmo::MODE GetMode() const { return mode; }

	void SetOperation(ImGuizmo::OPERATION o) { operation = o; }
	void SetMode(ImGuizmo::MODE m) { mode = m; }

private:

	void Draw(bool secondary = false) final;

	ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE mode = ImGuizmo::MODE::LOCAL;

	bool need_render = true;
};

#endif //!__SCENE_EDITOR_WINDOW__