#include "EditorWindow.h"
#include <MGL/Math/float4.h>

#include "SceneEditorWindow.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "RE_Time.h"
#include "RE_Command.h"
#include "RE_CameraManager.h"
#include "RE_CompTransform.h"

#include <SDL2/SDL_scancode.h>
#include <ImGui/imgui_internal.h>
#include <EASTL/bit.h>

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

	static int lastWidht = 0;
	static int lastHeight = 0;
	ImVec2 size = ImGui::GetWindowSize();
	width = static_cast<int>(size.x);
	heigth = static_cast<int>(size.y) - 28;
	if (recalc || lastWidht != width || lastHeight != heigth)
	{
		RE_INPUT->Push(RE_EventType::EDITORWINDOWCHANGED, RE_RENDER, RE_Cvar(lastWidht = width), RE_Cvar(lastHeight = heigth));
		RE_INPUT->Push(RE_EventType::EDITORWINDOWCHANGED, RE_EDITOR);
		recalc = false;
	}

	isWindowSelected = (ImGui::IsWindowHovered() && ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow));
	ImGui::SetCursorPos({ viewport.x, viewport.y });
	ImGui::Image(eastl::bit_cast<void*>(RE_RENDER->GetRenderViewTexture(RenderView::Type::EDITOR)), { viewport.z, viewport.w }, { 0.0, 1.0 }, { 1.0, 0.0 });

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
			RE_INPUT->Push(RE_EventType::EDITOR_SCENE_RAYCAST, RE_EDITOR, mousePosOnThis.x, mousePosOnThis.y);
	}

	GO_UID selected_uid = RE_EDITOR->GetSelected();
	if (selected_uid)
	{
		RE_GameObject* selected = RE_SCENE->GetGOPtr(selected_uid);
		RE_GameObject* parent = selected->GetParentPtr();
		RE_CompTransform* sTransform = selected->GetTransformPtr();

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
				RE_EDITOR->PushCommand(new RE_CMDTransformPosition(selected_uid, before, last));
				break;
			case ImGuizmo::ROTATE:
				RE_EDITOR->PushCommand(new RE_CMDTransformRotation(selected_uid, before, last));
				break;
			case ImGuizmo::SCALE:
				RE_EDITOR->PushCommand(new RE_CMDTransformScale(selected_uid, before, last));
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
