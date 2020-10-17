#include "RE_CompTransform.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "RE_Command.h"

#include "RE_GameObject.h"
#include "ImGui\imgui.h"
#include "RE_Math.h"

#define MIN_SCALE 0.001f

RE_CompTransform::RE_CompTransform() : RE_Component(C_TRANSFORM, nullptr)
{
}

RE_CompTransform::~RE_CompTransform()
{
	
}

void RE_CompTransform::SetUp(RE_GameObject* parent)
{
	go = parent;
	if (go) go->AddComponent(this);
	scale.scale = math::vec::one;

	if (go == nullptr)
		useParent = false;
}

void RE_CompTransform::SetUp(const RE_CompTransform& cmptransform, RE_GameObject* parent)
{
	go = parent;
	if (go) go->AddComponent(this);
	if (go == nullptr)
		useParent = false;

	SetPosition(cmptransform.pos);
	SetScale(cmptransform.scale.scale);
	SetRotation(cmptransform.rot_quat);
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

math::vec RE_CompTransform::GetRight()
{
	return model_global.Row3(0);
}

math::vec RE_CompTransform::GetLeft()
{
	return -model_global.Row3(0);
}

math::vec RE_CompTransform::GetUp()
{
	return model_global.Row3(1);
}

math::vec RE_CompTransform::GetDown()
{
	return -model_global.Row3(1);
}

math::vec RE_CompTransform::GetFront()
{
	return -model_global.Row3(2);
}

math::vec RE_CompTransform::GetBack()
{
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
		if (ImGui::DragFloat3("Position", p, 0.1f, -10000.f, 10000.f, "%.2f")) {
			
			if (!watchingChange) {
				watchingChange = true;
				before = pos;
				pFrom = true;
			}

			SetPosition({ p[0], p[1], p[2] });

			if (watchingChange) {
				frameWatched = true;
				last = pos;
			}

		}

		// Rotation -----------------------------------------------------
		float r[3] = { math::RadToDeg(rot_eul.x), math::RadToDeg(rot_eul.y), math::RadToDeg(rot_eul.z) };
		if (ImGui::DragFloat3("Rotation", r, 1.f, -360.f, 360.f, "%.2f")) {

			if (!watchingChange) {
				watchingChange = true;
				before = rot_eul;
				rFrom = true;
			}

			SetRotation({ math::DegToRad(r[0]), math::DegToRad(r[1]), math::DegToRad(r[2]) });

			if (watchingChange) {
				frameWatched = true;
				last = rot_eul;
			}
		}

		// Scale -----------------------------------------------------
		float s[3] = { scale.scale.x, scale.scale.y, scale.scale.z };
		if (ImGui::DragFloat3("Scale", s, 0.1f, -10000.f, 10000.f, "%.2f")) {

			if (!watchingChange) {
				watchingChange = true;
				before = scale.scale;
				sFrom = true;
			}

			SetScale({ s[0], s[1], s[2] });

			if (watchingChange) {
				frameWatched = true;
				last = scale.scale;
			}
		}

		if (watchingChange && (App->input->GetMouse().GetButton(1) == KEY_STATE::KEY_UP || !frameWatched)) {

			if (pFrom) {
				App->editor->PushCommand(new RE_CMDTransformPosition(go, before, last));
			}
			else if (rFrom) {
				App->editor->PushCommand(new RE_CMDTransformRotation(go, before, last));
			}
			else if (sFrom) {
				App->editor->PushCommand(new RE_CMDTransformScale(go, before, last));
			}
			
			watchingChange = false;
			before = math::vec::zero;
			last = math::vec::zero;
			pFrom = false;
			rFrom = false;
			sFrom = false;
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

