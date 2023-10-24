#include "EditorWindow.h"
#include <MGL/Math/float4.h>

#include "SceneEditorWindow.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "ModulePhysics.h"
#include "RE_CommandManager.h"
#include "ModuleRenderer3D.h"
#include "RE_FBOManager.h"
#include "RE_Time.h"
#include "RE_Profiler.h"
#include "RE_Command.h"
#include "RE_CameraManager.h"
#include "RE_CompTransform.h"
#include "RE_ResourceManager.h"

#include <ImGui/imgui_internal.h>
#include <GL/glew.h>

SceneEditorWindow::SceneEditorWindow() : OwnCameraRenderedWindow("Editor Scene", true)
{
	// Render View Setup
	render_view.flags =
		RenderView::Flag::FRUSTUM_CULLING |
		RenderView::Flag::OVERRIDE_CULLING |
		RenderView::Flag::OUTLINE_SELECTION |
		RenderView::Flag::SKYBOX |
		RenderView::Flag::BLENDED |
		RenderView::Flag::FACE_CULLING |
		RenderView::Flag::TEXTURE_2D |
		RenderView::Flag::COLOR_MATERIAL |
		RenderView::Flag::DEPTH_TEST;
	render_view.light_mode = RenderView::LightMode::DISABLED;
	render_view.fbos = {
		RE_FBOManager::CreateFBO(1024, 768, 1, true, true),
		RE_FBOManager::CreateDeferredFBO(1024, 768) };

	flags = 
		Flag::SELECT_GO_ON_MOUSE_CLICK |
		Flag::DRAW_DEBUG |
		Flag::DRAW_QUADTREE |
		Flag::DRAW_CAMERA_FRUSTUMS;

	all_aabb_color[0] = 0.f;
	all_aabb_color[1] = 1.f;
	all_aabb_color[2] = 0.f;

	sel_aabb_color[0] = 1.f;
	sel_aabb_color[1] = .5f;
	sel_aabb_color[2] = 0.f;

	quadtree_color[0] = 1.f;
	quadtree_color[1] = 1.f;
	quadtree_color[2] = 0.f;

	frustum_color[0] = 0.f;
	frustum_color[1] = 1.f;
	frustum_color[2] = 1.f;

	// Focus Camera
	auto root = RE_SCENE->GetRootCPtr();
	if (root) RE_EDITOR->SetSelected(root->GetFirstChildUID());
}

void SceneEditorWindow::DrawDebug() const
{
	RE_PROFILE(RE_ProfiledFunc::DrawDebug, RE_ProfiledClass::ModuleEditor)

	if (!HasFlag(Flag::DRAW_DEBUG)) return;

	RE_PHYSICS->DrawDebug(cam);

	// Check if any debug shape is rendered (aabb_drawing can be bypassed if none selected)
	auto selected = RE_EDITOR->GetSelected();
	auto mode = (selected ? aabb_drawing : static_cast<AABBDebugDrawing>(static_cast<int>(aabb_drawing) - 1));
	if (mode == AABBDebugDrawing::NONE &&
		!HasFlag(Flag::DRAW_QUADTREE) &&
		!HasFlag(Flag::DRAW_CAMERA_FRUSTUMS))
		return;

	ModuleRenderer3D::LoadCameraMatrixes(cam);

	glBegin(GL_LINES);

	DrawBoundingBoxes();
	DrawQuadTree();
	DrawFrustums();

	glEnd();

	if (grid->IsActive()) grid->Draw();
}

#pragma region Camera

void SceneEditorWindow::Orbit(float delta_x, float delta_y)
{
	auto selected = RE_EDITOR->GetSelected();
	if (!selected) return;

	cam.Orbit(
		cam_sensitivity * -delta_x,
		cam_sensitivity * delta_y,
		RE_SCENE->GetGOCPtr(selected)->GetGlobalBoundingBoxWithChilds().CenterPoint());
}

void SceneEditorWindow::Focus()
{
	auto selected = RE_EDITOR->GetSelected();
	if (!selected) return;

	math::AABB box = RE_SCENE->GetGOCPtr(selected)->GetGlobalBoundingBoxWithChilds();
	cam.Focus(box.CenterPoint(), box.HalfSize().Length());
}

#pragma endregion

void SceneEditorWindow::MousePick(float x, float y)
{
	float width, height;
	cam.GetTargetWidthHeight(width, height);

	GO_UID hit = RE_SCENE->RayCastGeometry(
		math::Ray(cam.GetFrustum().UnProjectLineSegment(
			(x - (width / 2.0f)) / (width / 2.0f),
			((height - y) - (height / 2.0f)) / (height / 2.0f))));

	if (hit) RE_EDITOR->SetSelected(hit);
}

void SceneEditorWindow::CheckboxFlag(const char* label, Flag flag)
{
	bool tmp = HasFlag(flag);
	if (ImGui::Checkbox(label, &tmp))
		tmp ? AddFlag(flag) : RemoveFlag(flag);
}

#pragma region Config Serialization

void SceneEditorWindow::Load3(RE_Json* node)
{
	// Flags
	node->PullBool("SelectMouseClick", true) ? AddFlag(Flag::SELECT_GO_ON_MOUSE_CLICK) : RemoveFlag(Flag::SELECT_GO_ON_MOUSE_CLICK);
	node->PullBool("FocusOnSelect", false) ? AddFlag(Flag::FOCUS_GO_ON_SELECT) : RemoveFlag(Flag::FOCUS_GO_ON_SELECT);
	node->PullBool("DebugDraw", true) ? AddFlag(Flag::DRAW_DEBUG) : RemoveFlag(Flag::DRAW_DEBUG);
	node->PullBool("QuadTree_Draw", true) ? AddFlag(Flag::DRAW_QUADTREE) : RemoveFlag(Flag::DRAW_QUADTREE);
	node->PullBool("Frustum_Draw", true) ? AddFlag(Flag::DRAW_CAMERA_FRUSTUMS) : RemoveFlag(Flag::DRAW_CAMERA_FRUSTUMS);

	// Debug Drawing
	aabb_drawing = static_cast<AABBDebugDrawing>(node->PullInt("AABB_Drawing", static_cast<int>(AABBDebugDrawing::SELECTED_ONLY)));
	math::vec color = node->PullFloatVector("AABB_Selected_Color", { 0.0f, 1.0f, 0.0f });
	memcpy_s(all_aabb_color, sizeof(float) * 3, color.ptr(), sizeof(float) * 3);
	color = node->PullFloatVector("AABB_Color", { 1.0f, 0.5f, 0.0f });
	memcpy_s(sel_aabb_color, sizeof(float) * 3, color.ptr(), sizeof(float) * 3);
	color = node->PullFloatVector("Quadtree_Color", { 1.0f, 1.0f, 0.0f });
	memcpy_s(quadtree_color, sizeof(float) * 3, color.ptr(), sizeof(float) * 3);
	color = node->PullFloatVector("Frustum_Color", { 0.0f, 1.0f, 1.0f });
	memcpy_s(frustum_color, sizeof(float) * 3, color.ptr(), sizeof(float) * 3);
}

void SceneEditorWindow::Save3(RE_Json* node) const
{
	// Flags
	node->Push("SelectMouseClick", HasFlag(Flag::SELECT_GO_ON_MOUSE_CLICK));
	node->Push("FocusOnSelect", HasFlag(Flag::FOCUS_GO_ON_SELECT));
	node->Push("DebugDraw", HasFlag(Flag::DRAW_DEBUG));
	node->Push("QuadTree_Draw", HasFlag(Flag::DRAW_QUADTREE));
	node->Push("Frustum_Draw", HasFlag(Flag::DRAW_CAMERA_FRUSTUMS));

	// Debug Drawing
	node->Push("AABB_Drawing", static_cast<int>(aabb_drawing));
	node->PushFloatVector("AABB_Selected_Color", { all_aabb_color[0], all_aabb_color[1], all_aabb_color[2] });
	node->PushFloatVector("AABB_Color", { sel_aabb_color[0], sel_aabb_color[1], sel_aabb_color[2] });
	node->PushFloatVector("Quadtree_Color", { quadtree_color[0], quadtree_color[1], quadtree_color[2] });
	node->PushFloatVector("Frustum_Color", { frustum_color[0], frustum_color[1], frustum_color[2] });
}

#pragma endregion

void SceneEditorWindow::Draw(bool secondary)
{
	need_render = ImGui::Begin(name, 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);
	if (!need_render)
	{
		ImGui::End();
		return;
	}

	if (secondary)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	UpdateWindow();

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();
	vMin.x += ImGui::GetWindowPos().x;
	vMin.y += ImGui::GetWindowPos().y;
	vMax.x += ImGui::GetWindowPos().x;
	vMax.y += ImGui::GetWindowPos().y;

	if (!ImGuizmo::IsOver() && !ImGuizmo::IsUsing() && isWindowSelected && !ImGui::GetKeyData(ImGuiKey::ImGuiKey_LeftAlt)->Down && ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Left))
	{
		ImVec2 mousePosOnThis = ImGui::GetMousePos();
		if ((mousePosOnThis.x -= vMin.x - ImGui::GetStyle().WindowPadding.x) > 0.f && (mousePosOnThis.y -= vMin.y - ImGui::GetStyle().WindowPadding.y) > 0.f)
			MousePick(mousePosOnThis.x, mousePosOnThis.y);
	}

	auto selected = RE_EDITOR->GetSelected();
	if (selected)
	{
		RE_GameObject* selected_go = RE_SCENE->GetGOPtr(selected);
		RE_GameObject* parent = selected_go->GetParentPtr();
		RE_CompTransform* sTransform = selected_go->GetTransformPtr();

		static float matA[16];
		static math::vec pos;
		static math::Quat rot, lastRot;
		static math::vec scl;

		static math::vec before = math::vec::zero, last = math::vec::zero;

		math::float4x4 cameraView = cam.GetView().Transposed();

		//filling matA
		sTransform->GetGlobalMatrix().Transposed().Decompose(pos, rot, scl);

		static bool watchingChange = false;
		if (!watchingChange)
		{
			switch (operation)
			{
			case ImGuizmo::TRANSLATE: before = sTransform->GetLocalPosition(); break;
			case ImGuizmo::ROTATE: before = sTransform->GetLocalEulerRotation(); break;
			case ImGuizmo::SCALE: before = sTransform->GetLocalScale(); break;
			}
		}

		bool isGlobal = mode == ImGuizmo::MODE::WORLD;
		if (isGlobal)
		{
			lastRot = rot;
			rot = math::Quat::identity;
		}

		ImGuizmo::RecomposeMatrixFromComponents(pos.ptr(), rot.ToEulerXYZ().ptr(), scl.ptr(), matA);

		math::float4x4 deltamatrix = math::float4x4::identity * RE_Time::DeltaTime();

		//SetRect of window at imgizmo
		ImGuizmo::SetRect(vMin.x, vMin.y, vMax.x - vMin.x, vMax.y - vMin.y);


		ImGuizmo::SetDrawlist();
		ImGuizmo::Manipulate(cameraView.ptr(), cam.GetProjection().Transposed().ptr(), operation, mode, matA, deltamatrix.ptr());

		if (ImGuizmo::IsUsing())
		{
			watchingChange = true;
			static float matrixTranslation[3], matrixRotation[3], matrixScale[3];
			ImGuizmo::DecomposeMatrixToComponents(matA, matrixTranslation, matrixRotation, matrixScale);

			math::float4x4 localMat = parent->GetTransformPtr()->GetGlobalMatrix().InverseTransposed();
			math::float4x4 globalMat = math::float4x4::FromTRS(
				math::vec(matrixTranslation),
				math::Quat::FromEulerXYZ(matrixRotation[0], matrixRotation[1], matrixRotation[2]),
				math::vec(matrixScale));

			localMat = localMat * globalMat;

			localMat.Decompose(pos, rot, scl);

			switch (operation)
			{
			case ImGuizmo::TRANSLATE:
				sTransform->SetPosition(pos);
				last = pos;
				break;
			case ImGuizmo::ROTATE:
				last = isGlobal ? (rot * lastRot).ToEulerXYZ() : rot.ToEulerXYZ();
				sTransform->SetRotation(last);
				break;
			case ImGuizmo::SCALE:
				sTransform->SetScale(scl);
				last = scl;
				break;
			}
		}

		if (watchingChange && ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Left))
		{
			switch (operation)
			{
			case ImGuizmo::TRANSLATE:
				RE_CommandManager::PushCommand(new RE_CMDTransformPosition(selected, before, last));
				break;
			case ImGuizmo::ROTATE:
				RE_CommandManager::PushCommand(new RE_CMDTransformRotation(selected, before, last));
				break;
			case ImGuizmo::SCALE:
				RE_CommandManager::PushCommand(new RE_CMDTransformScale(selected, before, last));
				break;
			}

			watchingChange = false;
			before = last = math::vec::zero;
		}
	}

	if (secondary)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	ImGui::End();
}

void SceneEditorWindow::DrawEditor3()
{
	ImGui::Separator();
	eastl::string id = name;

	// Selection Controls
	ImGui::PushID((id + " Select on mouse click").c_str());
	CheckboxFlag("Select on mouse click", Flag::SELECT_GO_ON_MOUSE_CLICK);
	ImGui::PopID();

	ImGui::PushID((id + " Focus on Select").c_str());
	CheckboxFlag("Focus on Select", Flag::FOCUS_GO_ON_SELECT);
	ImGui::PopID();

	// Debug Drawing
	ImGui::Separator();
	ImGui::PushID((id + " Debug Draw").c_str());
	CheckboxFlag("Debug Draw", Flag::DRAW_DEBUG);
	ImGui::PopID();

	if (!HasFlag(Flag::DRAW_DEBUG)) return;

	int aabb_d = static_cast<int>(aabb_drawing);
	ImGui::PushID((id + " Draw AABB").c_str());
	if (ImGui::Combo("Draw AABB", &aabb_d, "None\0Selected only\0All\0All w/ different selected\0"))
		aabb_drawing = static_cast<AABBDebugDrawing>(aabb_d);
	ImGui::PopID();

	if (aabb_drawing > AABBDebugDrawing::SELECTED_ONLY)
	{
		ImGui::PushID((id + " Color AABB").c_str());
		ImGui::ColorEdit3("Color AABB", all_aabb_color);
		ImGui::PopID();
	}
	if (static_cast<int>(aabb_drawing) % 2 == 1)
	{
		ImGui::PushID((id + " Color Selected").c_str());
		ImGui::ColorEdit3("Color Selected", sel_aabb_color);
		ImGui::PopID();
	}

	// QuadTrees
	ImGui::PushID((id + " Debug QuadTree").c_str());
	CheckboxFlag("Debug QuadTree", Flag::DRAW_QUADTREE);
	ImGui::PopID();
	if (HasFlag(Flag::DRAW_QUADTREE))
	{
		ImGui::PushID((id + " Color Quadtree").c_str());
		ImGui::ColorEdit3("Color Quadtree", quadtree_color);
		ImGui::PopID();
	}

	// Frustums
	ImGui::PushID((id + " Debug Camera Fustrums").c_str());
	CheckboxFlag("Debug Camera Fustrums", Flag::DRAW_CAMERA_FRUSTUMS);
	ImGui::PopID();
	if (HasFlag(Flag::DRAW_CAMERA_FRUSTUMS))
	{
		ImGui::PushID((id + " Color Fustrum").c_str());
		ImGui::ColorEdit3("Color Fustrum", frustum_color);
		ImGui::PopID();
	}
}

#pragma region Debug Drawing

void SceneEditorWindow::DrawBoundingBoxes() const
{
	switch (aabb_drawing)
	{
	case AABBDebugDrawing::SELECTED_ONLY:
	{
		glColor4f(sel_aabb_color[0], sel_aabb_color[1], sel_aabb_color[2], 1.0f);
		auto selected = RE_EDITOR->GetSelected();
		if (selected) RE_SCENE->GetGOCPtr(selected)->DrawGlobalAABB();
		break; 
	}
	case AABBDebugDrawing::ALL:
	{
		eastl::queue<const RE_GameObject*> objects;
		for (auto child : RE_SCENE->GetRootCPtr()->GetChildsPtr())
			objects.push(child);

		if (objects.empty()) break;

		glColor4f(all_aabb_color[0], all_aabb_color[1] * 255.0f, all_aabb_color[2], 1.0f);

		const RE_GameObject* object = nullptr;
		while (!objects.empty())
		{
			(object = objects.front())->DrawGlobalAABB();
			objects.pop();

			if (object->ChildCount() > 0)
				for (auto child : object->GetChildsPtr())
					objects.push(child);
		}

		break;
	}
	case AABBDebugDrawing::ALL_AND_SELECTED:
	{
		glColor4f(sel_aabb_color[0], sel_aabb_color[1], sel_aabb_color[2], 1.0f);
		auto selected = RE_EDITOR->GetSelected();
		RE_SCENE->GetGOCPtr(selected)->DrawGlobalAABB();

		eastl::queue<const RE_GameObject*> objects;
		for (auto child : RE_SCENE->GetRootCPtr()->GetChildsPtr())
			objects.push(child);

		if (!objects.empty())
		{
			glColor4f(all_aabb_color[0], all_aabb_color[1], all_aabb_color[2], 1.0f);

			while (!objects.empty())
			{
				const RE_GameObject* object = objects.front();
				objects.pop();

				if (object->GetUID() != selected) object->DrawGlobalAABB();

				if (object->ChildCount() > 0)
					for (auto child : object->GetChildsPtr())
						objects.push(child);
			}
		}

		break;
	}
	default: break;
	}
}

void SceneEditorWindow::DrawQuadTree() const
{
	if (!HasFlag(Flag::DRAW_QUADTREE)) return;

	glColor4f(quadtree_color[0], quadtree_color[1], quadtree_color[2], 1.0f);
	RE_SCENE->DrawSpacePartitioning();
}

void SceneEditorWindow::DrawFrustums() const
{
	if (!HasFlag(Flag::DRAW_CAMERA_FRUSTUMS)) return;

	glColor4f(frustum_color[0], frustum_color[1], frustum_color[2], 1.0f);
	for (auto& comp : RE_SCENE->GetScenePool()->GetAllCompCPtr(RE_Component::Type::CAMERA))
	{
		auto cam = dynamic_cast<const RE_CompCamera*>(comp);
		if (cam->draw_frustum) cam->Camera.DrawFrustum();
	}
}

#pragma endregion