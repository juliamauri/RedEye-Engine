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
	just_calculated_global = false;

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

void RE_CompTransform::SetPos(math::vec position)
{
	transform_modified = true;
	pos = position;
}

void RE_CompTransform::SetRot(math::vec euler)
{
	transform_modified = true;
	rot_eul = euler;
	math::vec rot_deg = math::DegToRad(rot_eul);
	rot_quat = math::Quat::FromEulerXYZ(rot_deg.x, rot_deg.y, rot_deg.z);
}

void RE_CompTransform::SetRot(math::Quat quat)
{
	transform_modified = true;
	rot_quat = quat;
	rot_eul = math::RadToDeg(rot_quat.ToEulerXYZ());;
}

void RE_CompTransform::SetScale(math::vec _scale)
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
	return front;
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

math::float4x4 RE_CompTransform::GetGlobalMatInvTrans() const
{
	return global_transform.InverseTransposed();
}

math::float4x4 RE_CompTransform::GetGlobalMatrix() const
{
	return global_transform;
}

void RE_CompTransform::LocalLookAt(math::vec& target_pos)
{
	math::vec direction = target_pos - pos;
	Quat::LookAt(front, direction.Normalized(), up, float3(0.f, 1.f, 0.f));

	SetRot(rot_quat * Quat::LookAt(front, direction.Normalized(), up, float3(0.f, 1.f, 0.f)));
}

void RE_CompTransform::LocalMove(Dir dir, float speed)
{
	if (speed != 0.f)
	{
		transform_modified = true;

		switch (dir)
		{
		case FORWARD:	pos += front * speed; break;
		case BACKWARD:	pos -= front * speed; break;
		case LEFT:		pos -= right * speed; break;
		case RIGHT:		pos += right * speed; break;
		}
	}
}

void RE_CompTransform::Orbit(float xoffset, float yoffset, math::vec& target)
{
	/*glm::vec3 focus = { target.x, target.y, target.z };
	float distance = glm::distance(Position, focus);
	Position = focus;

	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	if (Pitch > 89.0f) Pitch = 89.0f;
	if (Pitch < -89.0f) Pitch = -89.0f;

	Front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front.y = sin(glm::radians(Pitch));
	Front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(Front);

	Position -= Front * distance;

	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));

	Focus = Position + Front;*/
}

void RE_CompTransform::DrawProperties()
{
	if (ImGui::CollapsingHeader("Transform"))
	{
		// Position -----------------------------------------------------
		float p[3] = { pos.x, pos.y, pos.z };
		if (ImGui::InputFloat3("Position", p, 2))
			SetPos({ p[0], p[1], p[2] });

		// Rotation -----------------------------------------------------
		float r[3] = { rot_eul.x, rot_eul.y, rot_eul.z };
		if (ImGui::InputFloat3("Rotation", r, 2))
			SetRot({ r[0], r[1], r[2] });

		// Scale -----------------------------------------------------
		float s[3] = { scale.x, scale.y, scale.z };
		if (ImGui::InputFloat3("Scale", s, 2))
			SetScale({ s[0], s[1], s[2] });
	}
}

void RE_CompTransform::Reset()
{
	SetPos(math::vec::zero);
	SetRot(math::Quat::identity);
	SetScale(math::vec::one);
}

bool RE_CompTransform::HasChanged() const
{
	return just_calculated_global;
}

void RE_CompTransform::CalcGlobalTransform(bool call_transform_modified)
{
	if (go != nullptr)
	{
		RE_GameObject* parent = go->GetParent();

		if (parent != nullptr && parent->GetTransform() != nullptr)
		{
			global_transform = parent->GetTransform()->global_transform * local_transform;

			const std::list<RE_GameObject*> go_sons = go->GetChilds();

			if (!go_sons.empty())
			{
				std::list<RE_GameObject*>::const_iterator child = go_sons.begin();
				for (; child != go_sons.end(); child++)
					(*child)->GetTransform()->CalcGlobalTransform(call_transform_modified);
			}
		}
		else
		{
			global_transform = local_transform;
		}

		if (call_transform_modified) go->TransformModified();
	}
	else
	{
		global_transform = local_transform;
	}

	right = global_transform.Col3(0).Normalized();
	up = global_transform.Col3(1).Normalized();
	front = global_transform.Col3(2).Normalized();

	just_calculated_global = true;
}