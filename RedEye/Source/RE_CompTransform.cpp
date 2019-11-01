#include "RE_CompTransform.h"

#include "RE_GameObject.h"
#include "ImGui\imgui.h"
#include "RE_Math.h"

#define MIN_SCALE 0.001f

RE_CompTransform::RE_CompTransform(RE_GameObject * go) : RE_Component(C_TRANSFORM, go) 
{
	scale.scale = math::vec::one;

	if (go == nullptr)
		useParent = false;
}

RE_CompTransform::RE_CompTransform(RE_CompTransform & cmptransform, RE_GameObject * go) : RE_Component(C_TRANSFORM, go)
{
	if (go == nullptr)
		useParent = false;

	SetPosition(cmptransform.pos);
	SetScale(cmptransform.scale.scale);
	SetRotation(cmptransform.rot_quat);
}

RE_CompTransform::~RE_CompTransform()
{
	
}

void RE_CompTransform::Update()
{
	if (needed_update_transform)
		CalcGlobalTransform();
	else
		ConfirmChange();
}

math::float4x4 RE_CompTransform::GetLocalMatrixModel()
{
	if (needed_update_transform)
		model_local = math::float4x4::FromTRS(pos, rot_quat, scale.scale);

	return model_local;
}

math::float4x4 RE_CompTransform::GetMatrixModel()
{
	if (needed_update_transform)
		CalcGlobalTransform();

	return model_global;
}

float* RE_CompTransform::GetShaderModel()
{
	if (needed_update_transform)
		CalcGlobalTransform();

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

void RE_CompTransform::SetScale(math::vec scale)
{
	this->scale.scale = scale;
	/*this->scale.scale = {
	this->scale.scale.x > MIN_SCALE ? 1.0f / this->scale.scale.x : 1.0f / MIN_SCALE,
	this->scale.scale.y > MIN_SCALE ? 1.0f / this->scale.scale.y : 1.0f / MIN_SCALE,
	this->scale.scale.z > MIN_SCALE ? 1.0f / this->scale.scale.z : 1.0f / MIN_SCALE };*/

	needed_update_transform = true;
}

void RE_CompTransform::SetPosition(math::vec position)
{
	pos = position;
	needed_update_transform = true;
}

void RE_CompTransform::SetGlobalPosition(math::vec global_position)
{
	pos = (go != nullptr && go->GetParent() != nullptr) ? global_position - go->GetParent()->GetTransform()->GetGlobalPosition() : global_position;
	needed_update_transform = true;
}

math::Quat RE_CompTransform::GetLocalQuaternionRotation() const
{
	return rot_quat;
}

math::vec RE_CompTransform::GetLocalEulerRotation() const
{
	return rot_eul;
}

math::vec RE_CompTransform::GetLocalScale() const
{
	return scale.scale;
}

math::vec RE_CompTransform::GetLocalPosition() const
{
	return pos;
}

math::vec RE_CompTransform::GetGlobalPosition()
{
	if (needed_update_transform)
		CalcGlobalTransform();

	return model_global.Row3(3);
}

void RE_CompTransform::DrawProperties()
{
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Position -----------------------------------------------------
		float p[3] = { pos.x, pos.y, pos.z };
		if (ImGui::DragFloat3("Position", p, 0.1f, -10000.f, 10000.f, "%.2f"))
		{
			SetPosition({ p[0], p[1], p[2] });
			go->SetStatic(false);
		}

		// Rotation -----------------------------------------------------
		float r[3] = { math::RadToDeg(rot_eul.x), math::RadToDeg(rot_eul.y), math::RadToDeg(rot_eul.z) };
		if (ImGui::DragFloat3("Rotation", r, 1.f, -360.f, 360.f, "%.2f"))
		{
			SetRotation({ math::DegToRad(r[0]), math::DegToRad(r[1]), math::DegToRad(r[2]) });
			go->SetStatic(false);
		}

		// Scale -----------------------------------------------------
		float s[3] = { scale.scale.x, scale.scale.y, scale.scale.z };
		if (ImGui::DragFloat3("Scale", s, 0.1f, -10000.f, 10000.f, "%.2f"))
		{
			SetScale({ s[0], s[1], s[2] });
			go->SetStatic(false);
		}
	}
}

bool RE_CompTransform::HasChanged() const
{
	return has_changed;
}

void RE_CompTransform::ConfirmChange()
{
	has_changed = false;
}

void RE_CompTransform::OnTransformModified()
{
	if (!has_changed)
		needed_update_transform = true;
}

void RE_CompTransform::CalcGlobalTransform()
{
	model_local = math::float4x4::FromTRS(pos, rot_quat, scale.scale);

	model_local.Transpose();

	if (useParent && go->GetParent() != nullptr)
		model_global = model_local * go->GetParent()->GetTransform()->GetMatrixModel();
	else
		model_global = model_local;

	needed_update_transform = false;
	has_changed = true;

	if (go != nullptr)
		go->TransformModified();
}

