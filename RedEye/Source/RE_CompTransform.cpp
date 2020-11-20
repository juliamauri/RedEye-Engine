#include "RE_CompTransform.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "RE_Command.h"

#include "RE_GameObject.h"
#include "RE_GOManager.h"
#include "ImGui\imgui.h"
#include "RE_Math.h"

#define MIN_SCALE 0.001f

RE_CompTransform::RE_CompTransform() : RE_Component(C_TRANSFORM)
{
	scale.scale = math::vec::one;
}

RE_CompTransform::~RE_CompTransform() {}

void RE_CompTransform::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent)
{
	pool_gos = pool;
	if (useParent = (go = parent)) pool_gos->AtPtr(go)->ReportComponent(id, type);

	RE_CompTransform* t = dynamic_cast<RE_CompTransform*>(copy);
	SetPosition(t->pos);
	SetScale(t->scale.scale);
	SetRotation(t->rot_quat);
}

void RE_CompTransform::Update()
{
	if (needed_update_transform) CalcGlobalTransform();
}

bool RE_CompTransform::CheckUpdate()
{
	bool ret = needed_update_transform;
	if (needed_update_transform) CalcGlobalTransform();
	return ret;
}

math::float4x4 RE_CompTransform::GetLocalMatrix()
{
	if (needed_update_transform) model_local = math::float4x4::FromTRS(pos, rot_quat, scale.scale);
	return model_local;
}

math::float4x4 RE_CompTransform::GetGlobalMatrix()
{
	if (needed_update_transform) CalcGlobalTransform();
	return model_global;
}

const float* RE_CompTransform::GetGlobalMatrixPtr()
{
	if (needed_update_transform) CalcGlobalTransform();
	return model_global.ptr();
}

void RE_CompTransform::SetRotation(math::Quat rotation)
{
	rot_quat = rotation;
	rot_eul = rotation.ToEulerXYZ();
	rot_mat = rotation.ToFloat3x3();
	needed_update_transform = true;
}

void RE_CompTransform::SetRotation(math::vec rotation)
{
	rot_eul = rotation;
	rot_quat = math::Quat::FromEulerXYZ(rotation.x, rotation.y, rotation.z);
	rot_mat = math::float3x3::FromEulerXYZ(rotation.x, rotation.y, rotation.z);
	needed_update_transform = true;
}

void RE_CompTransform::SetRotation(math::float3x3 rotation)
{
	rot_mat = rotation;
	rot_eul = rotation.ToEulerXYZ();
	rot_quat = rotation.ToQuat();
	needed_update_transform = true;
}

void RE_CompTransform::SetScale(math::vec _scale)
{
	scale.scale = _scale;
	needed_update_transform = true;
}

void RE_CompTransform::SetPosition(math::vec position)
{
	pos = position;
	needed_update_transform = true;
}

void RE_CompTransform::SetGlobalPosition(math::vec global_position)
{
	pos = global_position;

	if (go)
	{
		const RE_GameObject* p = pool_gos->AtCPtr(go)->GetParentCPtr();
		if (p != nullptr)
		{
			pos -= p->GetTransformPtr()->GetGlobalPosition();
		}
	}
	needed_update_transform = true;
}

math::Quat RE_CompTransform::GetLocalQuaternionRotation() const { return rot_quat; }
math::vec RE_CompTransform::GetLocalEulerRotation() const { return rot_eul; }
math::vec RE_CompTransform::GetLocalScale() const { return scale.scale; }
math::vec RE_CompTransform::GetLocalPosition() const { return pos; }

math::vec RE_CompTransform::GetGlobalPosition()
{
	if (needed_update_transform) CalcGlobalTransform();
	return model_global.Row3(3);
}

math::vec RE_CompTransform::GetRight()
{
	if (needed_update_transform) CalcGlobalTransform();
	return model_global.Row3(0);
}

math::vec RE_CompTransform::GetLeft()
{
	if (needed_update_transform) CalcGlobalTransform();
	return -model_global.Row3(0);
}

math::vec RE_CompTransform::GetUp()
{
	if (needed_update_transform) CalcGlobalTransform();
	return model_global.Row3(1);
}

math::vec RE_CompTransform::GetDown()
{
	if (needed_update_transform) CalcGlobalTransform();
	return -model_global.Row3(1);
}

math::vec RE_CompTransform::GetFront()
{
	if (needed_update_transform) CalcGlobalTransform();
	return -model_global.Row3(2);
}

math::vec RE_CompTransform::GetBack()
{
	if (needed_update_transform) CalcGlobalTransform();
	return model_global.Row3(2);
}

void RE_CompTransform::DrawProperties()
{
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static bool watchingChange = false;
		static bool frameWatched = false;
		static math::vec before = math::vec::zero;
		static math::vec last = math::vec::zero;

		static bool pFrom = false;
		static bool rFrom = false;
		static bool sFrom = false;

		// Position -----------------------------------------------------
		float p[3] = { pos.x, pos.y, pos.z };
		if (ImGui::DragFloat3("Position", p, 0.1f, -10000.f, 10000.f, "%.2f"))
		{
			if (!watchingChange)
			{
				watchingChange = true;
				before = pos;
				pFrom = true;
			}

			SetPosition({ p[0], p[1], p[2] });

			if (watchingChange)
			{
				frameWatched = true;
				last = pos;
			}
		}

		// Rotation -----------------------------------------------------
		float r[3] = { math::RadToDeg(rot_eul.x), math::RadToDeg(rot_eul.y), math::RadToDeg(rot_eul.z) };
		if (ImGui::DragFloat3("Rotation", r, 1.f, -360.f, 360.f, "%.2f"))
		{
			if (!watchingChange)
			{
				watchingChange = true;
				before = rot_eul;
				rFrom = true;
			}

			SetRotation({ math::DegToRad(r[0]), math::DegToRad(r[1]), math::DegToRad(r[2]) });

			if (watchingChange)
			{
				frameWatched = true;
				last = rot_eul;
			}
		}

		// Scale -----------------------------------------------------
		float s[3] = { scale.scale.x, scale.scale.y, scale.scale.z };
		if (ImGui::DragFloat3("Scale", s, 0.1f, -10000.f, 10000.f, "%.2f"))
		{
			if (!watchingChange)
			{
				watchingChange = true;
				before = scale.scale;
				sFrom = true;
			}

			SetScale({ s[0], s[1], s[2] });

			if (watchingChange)
			{
				frameWatched = true;
				last = scale.scale;
			}
		}

		if (watchingChange && (App::input->GetMouse().GetButton(1) == KEY_STATE::KEY_UP || !frameWatched))
		{
			if		(pFrom) App::editor->PushCommand(new RE_CMDTransformPosition(go, before, last));
			else if (rFrom) App::editor->PushCommand(new RE_CMDTransformRotation(go, before, last));
			else if (sFrom) App::editor->PushCommand(new RE_CMDTransformScale(go, before, last));
			
			watchingChange = pFrom = rFrom = sFrom = false;
			before = last = math::vec::zero;
		}
	}
}

void RE_CompTransform::OnTransformModified() { needed_update_transform = true; }

math::float4x4 RE_CompTransform::UpdateGlobalMatrixFromParent(math::float4x4 parent)
{
	needed_update_transform = false;
	model_local = math::float4x4::FromTRS(pos, rot_quat, scale.scale).Transposed();
	return model_global = model_local * parent;
}

void RE_CompTransform::CalcGlobalTransform()
{
	needed_update_transform = false;
	model_local = math::float4x4::FromTRS(pos, rot_quat, scale.scale).Transposed();

	if (useParent)
	{
		RE_GameObject* go_ptr = pool_gos->AtPtr(go);
		const RE_GameObject* parent = go_ptr->GetParentCPtr();
		if (parent != nullptr)
		{
			math::float4x4 next_global = model_local * parent->GetTransformPtr()->GetGlobalMatrix();
			if (!next_global.Equals(model_global))
				Event::Push(TRANSFORM_MODIFIED, App::scene, go, model_global = next_global);
		}
	}
	else model_global = model_local;
}

