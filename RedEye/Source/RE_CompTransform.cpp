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

	front = math::vec(0.0f, 0.0f, 1.0f);
	up = math::vec(0.0f,1.0f,0.0f);
	right = math::vec(1.0f,0.0f,0.0f);
}

RE_CompTransform::~RE_CompTransform()
{
}

void RE_CompTransform::Update()
{
	if (needed_update_transform)
		CalcGlobalTransform();
}

math::float4x4 RE_CompTransform::GetMatrixModel()
{
	return model_global;
}

float* RE_CompTransform::GetShaderModel()
{
	return model_global.ptr();
}

void RE_CompTransform::SetRotation(math::Quat rotation)
{
	rot_quat = rotation;
	using_euler = false;
	needed_update_transform = true;
}

void RE_CompTransform::SetRotation(math::vec rotation)
{
	rot_eul = rotation;
	using_euler = true;
	needed_update_transform = true;
}

void RE_CompTransform::SetScale(math::vec scale)
{
	this->scale.scale = scale;
	this->scale.scale = {
	this->scale.scale.x > MIN_SCALE ? 1.0f / this->scale.scale.x : 1.0f / MIN_SCALE,
	this->scale.scale.y > MIN_SCALE ? 1.0f / this->scale.scale.y : 1.0f / MIN_SCALE,
	this->scale.scale.z > MIN_SCALE ? 1.0f / this->scale.scale.z : 1.0f / MIN_SCALE };

	needed_update_transform = true;
}

void RE_CompTransform::SetPosition(math::vec position)
{
	pos = position;
	needed_update_transform = true;
}

math::Quat RE_CompTransform::GetQuaternionRotation()
{
	return rot_quat;
}

math::vec RE_CompTransform::GetEulerRotation()
{
	return rot_eul;
}

math::vec RE_CompTransform::GetScale()
{
	return scale.scale;
}

math::vec RE_CompTransform::GetPosition()
{
	return pos;
}

math::vec RE_CompTransform::GetGlobalPosition()
{
	return model_global.Col3(3);
}

void RE_CompTransform::LocalMove(Dir dir, float speed)
{
	if (speed != 0.f)
	{
		needed_update_transform = true;

		switch (dir)
		{
		case FORWARD:	pos -= front * speed; break;
		case BACKWARD:	pos += front * speed; break;
		case LEFT:		pos -= right * speed; break;
		case RIGHT:		pos += right * speed; break;
		case UP:		pos += up * speed; break;
		case DOWN:		pos -= up * speed; break;
		}
	}
}

void RE_CompTransform::DrawProperties()
{
	if (ImGui::CollapsingHeader("Transform"))
	{
		// Position -----------------------------------------------------
		float p[3] = { pos.x, pos.y, pos.z };
		if (ImGui::DragFloat3("Position", p, 0.1f, -10000.f, 10000.f, "%.2f"))
			SetPosition({ p[0], p[1], p[2] });

		// Rotation -----------------------------------------------------
		float r[3] = { rot_eul.x, rot_eul.y, rot_eul.z };
		if (ImGui::DragFloat3("Rotation", r, 0.1f, -360.f, 360.f, "%.2f"))
			SetRotation({ r[0], r[1], r[2] });

		// Scale -----------------------------------------------------
		float s[3] = { scale.scale.x, scale.scale.y, scale.scale.z };
		if (ImGui::InputFloat3("Scale", s, 2))
			SetScale({ s[0], s[1], s[2] });
	}
}

bool RE_CompTransform::HasChanged()
{
	has_changed = false;
	return !has_changed;
}

void RE_CompTransform::CalcGlobalTransform()
{
	model_local = math::float4x4::identity;

	model_local = model_local * scale;

	if (using_euler)
	{
		model_local = model_local * math::float4x4::RotateX(math::DegToRad(rot_eul.x));
		model_local = model_local * math::float4x4::RotateY(math::DegToRad(rot_eul.y));
		model_local = model_local * math::float4x4::RotateZ(math::DegToRad(rot_eul.z));
	}
	else
		model_local = model_local * math::float4x4(rot_quat);

	model_local = model_local * math::float4x4::Translate(pos);

	right = model_local.Col3(0); right.Normalize();
	up = model_local.Col3(1); up.Normalize();
	front = model_local.Col3(2); front.Normalize();

	if (useParent)
		model_global = go->GetTransform()->GetMatrixModel() * model_local;
	else
		model_global = model_local;

	needed_update_transform = false;
	has_changed = true;
}
