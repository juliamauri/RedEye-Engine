#include "RE_CompTransform.h"

#include "RE_GameObject.h"
#include "RE_CompCamera.h"
#include "ImGui\imgui.h"

#define MIN_SCALE 0.001f

RE_CompTransform::RE_CompTransform(RE_GameObject* go, math::vec pos) :
	RE_Component(C_TRANSFORM, go)
{
	CalcGlobalTransform(false);
}

RE_CompTransform::~RE_CompTransform()
{}

void RE_CompTransform::Update()
{
	if (transform_modified)
	{
		// make sure scale is not 0
		math::vec real_scale = {
			scale.x > MIN_SCALE ? 1.0f / scale.x : 1.0f / MIN_SCALE,
			scale.y > MIN_SCALE ? 1.0f / scale.y : 1.0f / MIN_SCALE,
			scale.z > MIN_SCALE ? 1.0f / scale.z : 1.0f / MIN_SCALE };

		local_transform = math::float4x4::FromTRS(pos.Neg(), rot_quat, real_scale);
		CalcGlobalTransform();
		transform_modified = false;
	}
}

void RE_CompTransform::SetPos(const math::vec position)
{
	transform_modified = true;
	pos = position;
}

void RE_CompTransform::SetRot(const math::vec euler)
{
	transform_modified = true;
	rot_eul = euler;
	math::vec rot_deg = math::DegToRad(rot_eul);
	rot_quat = math::Quat::FromEulerXYZ(rot_deg.x, rot_deg.y, rot_deg.z);
}

void RE_CompTransform::SetRot(const math::Quat quat)
{
	transform_modified = true;
	rot_quat = quat;
	rot_eul = math::RadToDeg(rot_quat.ToEulerXYZ());;
}

void RE_CompTransform::SetScale(const math::vec _scale)
{
	transform_modified = true;
	scale = _scale;
}

math::vec RE_CompTransform::GetPosition() const
{
	return pos;
}

math::vec RE_CompTransform::GetRight() const
{
	return right;
}

math::vec RE_CompTransform::GetUp() const
{
	return up;
}

math::vec RE_CompTransform::GetForward() const
{
	return forward;
}

math::vec RE_CompTransform::GetRotXYZ() const
{
	return rot_eul;
}

math::Quat RE_CompTransform::GetRot() const
{
	return rot_quat;
}

math::vec RE_CompTransform::GetScale() const
{
	return scale;
}

math::float4x4 RE_CompTransform::GetLocalMatrix() const
{
	return local_transform.InverseTransposed();
}

math::float4x4 RE_CompTransform::GetGlobalMatrix() const
{
	return global_transform.InverseTransposed();
}

void RE_CompTransform::LookAt(math::vec cameraTarget)
{

}

void RE_CompTransform::DrawProperties()
{
	// Position -----------------------------------------------------
	float holder = pos.x;
	if (ImGui::SliderFloat("Pos X", &holder, -10.f, 10.f, "%.3f"))
	{
		pos.x = holder;
		transform_modified = true;
	}
	holder = pos.y;
	if (ImGui::SliderFloat("Pos Y", &holder, -10.f, 10.f, "%.3f"))
	{
		pos.y = holder;
		transform_modified = true;
	}
	holder = pos.z;
	if (ImGui::SliderFloat("Pos Z", &holder, -10.f, 10.f, "%.3f"))
	{
		pos.z = holder;
		transform_modified = true;
	}

	bool allow_edit = true;

	if (allow_edit)
	{
		// Rotation -----------------------------------------------------
		holder = rot_eul.x;
		if (ImGui::SliderFloat("Rot X", &holder, MIN_SCALE, 360.f, "%.1f"))
		{
			rot_eul.x = holder;
			SetRot(rot_eul);
		}
		holder = rot_eul.y;
		if (ImGui::SliderFloat("Rot Y", &holder, MIN_SCALE, 360.f, "%.1f"))
		{
			rot_eul.y = holder;
			SetRot(rot_eul);
		}
		holder = rot_eul.z;
		if (ImGui::SliderFloat("Rot Z", &holder, MIN_SCALE, 360.f, "%.1f"))
		{
			rot_eul.z = holder;
			SetRot(rot_eul);
		}

		// Scale -----------------------------------------------------
		holder = scale.x;
		if (ImGui::SliderFloat("Scale X", &holder, 0.f, 10.f, "%.1f"))
		{
			scale.x = holder;
			transform_modified = true;
		}
		holder = scale.y;
		if (ImGui::SliderFloat("Scale Y", &holder, 0.f, 10.f, "%.1f"))
		{
			scale.y = holder;
			transform_modified = true;
		}
		holder = scale.z;
		if (ImGui::SliderFloat("Scale Z", &holder, 0.f, 10.f, "%.1f"))
		{
			scale.z = holder;
			transform_modified = true;
		}
	}
	else
	{
		ImGui::Text("Rot X: %f", rot_eul.x);
		ImGui::Text("Rot Y: %f", rot_eul.y);
		ImGui::Text("Rot Z: %f", rot_eul.z);
		ImGui::Text("Scale X: %f", scale.x);
		ImGui::Text("Scale Y: %f", scale.y);
		ImGui::Text("Scale Z: %f", scale.z);
	}
}

void RE_CompTransform::Reset()
{
	SetPos(math::vec::zero);
	SetRot(math::Quat::identity);
	SetScale(math::vec::one);
}

void RE_CompTransform::CalcGlobalTransform(bool call_tranf_modified)
{
	if (go != nullptr)
	{
		RE_GameObject* parent = go->GetParent();

		if (parent != nullptr && parent->transform != nullptr)
		{
			global_transform = parent->transform->global_transform * local_transform;

			const std::list<RE_GameObject*>* go_sons = go->GetChilds();

			if (!go_sons->empty())
			{
				std::list<RE_GameObject*>::const_iterator child = go_sons->begin();
				for (; child != go_sons->end(); child++)
					(*child)->transform->CalcGlobalTransform();
			}
		}
		else
		{
			global_transform = local_transform;
		}

		right = global_transform.Col3(0).Normalized();
		up = global_transform.Col3(1).Normalized();
		forward = global_transform.Col3(2).Normalized();

		if (call_tranf_modified) go->TransformModified();
	}
	else if(update_camera != nullptr)
	{
		global_transform = local_transform;

		right = global_transform.Col3(0).Normalized();
		up = global_transform.Col3(1).Normalized();
		forward = global_transform.Col3(2).Normalized();

		update_camera->OnTransformModified();
	}
}

void RE_CompTransform::setCamera(RE_CompCamera * cam)
{
	update_camera = cam;
}
