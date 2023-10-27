#ifndef __SCENE_EDITOR_WINDOW__
#define __SCENE_EDITOR_WINDOW__

#include "RenderedWindow.h"
#include <ImGuiWidgets/ImGuizmo/ImGuizmo.h>

class SceneEditorWindow :public OwnCameraRenderedWindow
{
public:

	enum Flag
	{
		SELECT_GO_ON_MOUSE_CLICK = 1 << 0,
		FOCUS_GO_ON_SELECT = 1 << 1,

		DRAW_DEBUG = 1 << 2,
		DRAW_QUADTREE = 1 << 3,
		DRAW_CAMERA_FRUSTUMS = 1 << 4
	};

private:

	enum class AABBDebugDrawing : int
	{
		NONE = 0,
		SELECTED_ONLY,
		ALL,
		ALL_AND_SELECTED,
	};

	// Flags
	ushort flags = 0;

	// Debug Drawing
	AABBDebugDrawing aabb_drawing = AABBDebugDrawing::ALL_AND_SELECTED;
	float all_aabb_color[3];
	float sel_aabb_color[3];
	float quadtree_color[3];
	float frustum_color[3];

	// Guizmos
	ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE mode = ImGuizmo::MODE::LOCAL;

public:

	SceneEditorWindow();
	~SceneEditorWindow() final = default;

	void DrawDebug() const final;
	void DrawOther() const final;

	// Guizmos
	ImGuizmo::OPERATION GetOperation() const { return operation; }
	ImGuizmo::MODE GetMode() const { return mode; }
	void SetOperation(ImGuizmo::OPERATION o) { operation = o; }
	void SetMode(ImGuizmo::MODE m) { mode = m; }

	// Camera
	void Orbit(float delta_x, float delta_y) final;
	void Focus() final;
	const math::Frustum* GetFrustum() const final;

	// GO Selection
	void MousePick(float x, float y);

	// Flags
	inline void AddFlag(Flag flag) { flags |= static_cast<ushort>(flag); }
	inline void RemoveFlag(Flag flag) { flags -= static_cast<ushort>(flag); }
	inline const bool HasFlag(Flag flag) const { return flags & static_cast<ushort>(flag); }
	void CheckboxFlag(const char* label, Flag flag);

protected:

	// Serialization
	void Load3(RE_Json* node) final;
	void Save3(RE_Json* node) const final;

private:

	void Draw(bool secondary = false) final;
	void DrawEditor3() final;

	// Debug Drawing
	void DrawBoundingBoxes() const;
	void DrawQuadTree() const;
	void DrawFrustums() const;
};

#endif //!__SCENE_EDITOR_WINDOW__