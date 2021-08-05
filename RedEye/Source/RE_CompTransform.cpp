#include "RE_CompTransform.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "RE_Command.h"

#include "RE_GameObject.h"
#include "RE_ECS_Pool.h"
#include "ImGui\imgui.h"

#define MIN_SCALE 0.001f

RE_CompTransform::RE_CompTransform() : RE_Component(C_TRANSFORM)
{
	scale.scale = math::vec::one;
}

RE_CompTransform::~RE_CompTransform() {}

void RE_CompTransform::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent)
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
	math::vec previous_pos = pos;
	pos = global_position;

	if (useParent)
	{
		const RE_GameObject* p = pool_gos->AtCPtr(go)->GetParentCPtr();
		if (p != nullptr)
		{
			pos -= p->GetTransformPtr()->GetGlobalPosition();
		}
	}

	if (!pos.Equals(previous_pos)) needed_update_transform = true;
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

void RE_CompTransform::LocalPan(float rad_dx, float rad_dy, float rad_dz)
{
	rot_eul -= math::vec(rad_dy * -1, rad_dx, rad_dz);
	rot_quat = math::Quat::FromEulerXYZ(rot_eul.x, rot_eul.y, rot_eul.z);
	rot_mat = rot_quat.ToFloat3x3();
	needed_update_transform = true;
}

void RE_CompTransform::LocalMove(Dir dir, float speed)
{
	if (speed != 0.f)
	{
		switch (dir) {
		case FORWARD:	pos -= model_global.WorldZ() * speed; break;
		case BACKWARD:	pos += model_global.WorldZ() * speed; break;
		case LEFT:		pos -= model_global.WorldX() * speed; break;
		case RIGHT:		pos += model_global.WorldX() * speed; break;
		case UP:		pos += model_global.WorldY() * speed; break;
		case DOWN:		pos -= model_global.WorldY() * speed; break; }

		needed_update_transform = true;
	}
}

void RE_CompTransform::Orbit(float rad_dx, float rad_dy, const math::vec center)
{
	if (rad_dx != 0.f || rad_dy != 0.f)
	{
		bool has_parent = useParent;
		if (has_parent)
		{
			const RE_GameObject* go_ptr = nullptr;
			go_ptr = pool_gos->AtCPtr(go);
			if (has_parent = go_ptr->GetParentUID())
			{
				math::float4x4 parent_global = go_ptr->GetParentCPtr()->GetTransformPtr()->GetGlobalMatrix();

				if (needed_update_transform) model_local = math::float4x4::FromTRS(pos, rot_quat, scale.scale).Transposed();
				model_global = model_local * parent_global;

				float distance = model_global.Row3(3).Distance(center);

				LocalPan(rad_dx, rad_dy);
				pos = center - parent_global.Row3(3) + ((math::float4x4::FromTRS(pos, rot_quat, scale.scale).Transposed() * parent_global).Row3(2).Normalized() * distance);
			}
		}

		if (!has_parent)
		{
			LocalPan(rad_dx, rad_dy);
			pos = center + (math::float4x4::FromTRS(pos, rot_quat, scale.scale).Row3(2).Normalized() * pos.Distance(center));
		}
	}
}

void RE_CompTransform::Focus(const math::vec center, float v_fov_rads, float h_fov_rads, float radius, float min_dist)
{
	if (radius > 0)
	{
		float camDistance = min_dist;

		// Vertical distance
		float v_dist = radius / math::Sin(v_fov_rads / 2.0f);
		if (v_dist > camDistance) camDistance = v_dist;

		// Horizontal distance
		float h_dist = radius / math::Sin(h_fov_rads / 2.0f);
		if (h_dist > camDistance) camDistance = h_dist;

		bool has_parent = useParent;
		if (has_parent)
		{
			const RE_GameObject* go_ptr = nullptr;
			go_ptr = pool_gos->AtCPtr(go);
			if (has_parent = go_ptr->GetParentUID())
			{
				math::float4x4 parent_global = go_ptr->GetParentCPtr()->GetTransformPtr()->GetGlobalMatrix();

				if (needed_update_transform) model_local = math::float4x4::FromTRS(pos, rot_quat, scale.scale).Transposed();
				model_global = model_local * parent_global;

				pos = (center + (model_global.Col3(2).Normalized() * camDistance)) - parent_global.Row3(3);
			}
		}

		if (!has_parent)
		{
			if (needed_update_transform) model_local = math::float4x4::FromTRS(pos, rot_quat, scale.scale).Transposed();
			pos = center + (model_local.Col3(2).Normalized() * camDistance);
		}

		needed_update_transform = true;
	}

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
		bool frameWatched = false;
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

		if (watchingChange && (RE_INPUT->GetMouse().GetButton(1) == KEY_STATE::KEY_UP || !frameWatched))
		{
			if		(pFrom) RE_EDITOR->PushCommand(new RE_CMDTransformPosition(go, before, last));
			else if (rFrom) RE_EDITOR->PushCommand(new RE_CMDTransformRotation(go, before, last));
			else if (sFrom) RE_EDITOR->PushCommand(new RE_CMDTransformScale(go, before, last));
			
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
				RE_INPUT->Push(RE_EventType::TRANSFORM_MODIFIED, RE_SCENE, go, model_global = next_global);
		}
	}
	else model_global = model_local;
}

