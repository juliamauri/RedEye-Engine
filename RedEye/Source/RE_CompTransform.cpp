#include "RE_CompTransform.h"

#include "RE_GameObject.h"
#include "RE_CompCamera.h"
#include "ImGui\imgui.h"

#define MIN_SCALE 0.001f

RE_CompTransform::RE_CompTransform(RE_GameObject* go) :
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
		scale = {
			scale.x > MIN_SCALE ? 1.0f / scale.x : 1.0f / MIN_SCALE,
			scale.y > MIN_SCALE ? 1.0f / scale.y : 1.0f / MIN_SCALE,
			scale.z > MIN_SCALE ? 1.0f / scale.z : 1.0f / MIN_SCALE };

		CalcGlobalTransform();
		transform_modified = false;
	}
}

void RE_CompTransform::SetPos(math::vec position)
{
	transform_modified = true;
	pos = position;
}

void RE_CompTransform::SetLocalRot(math::vec euler)
{
	transform_modified = true;
	rot_eul = euler;
	math::vec rot_deg = math::DegToRad(rot_eul);
	rot_quat = math::Quat::FromEulerXYZ(rot_deg.x, rot_deg.y, rot_deg.z);
}

void RE_CompTransform::SetLocalRot(math::Quat quat)
{
	transform_modified = true;
	rot_quat = quat;
	rot_eul = math::RadToDeg(rot_quat.ToEulerXYZ());
}

void RE_CompTransform::SetScale(math::vec _scale)
{
	transform_modified = true;
	scale = _scale;
}

void RE_CompTransform::LocalRotate(math::vec axis, float angle)
{
	rot_quat = rot_quat.RotateAxisAngle(axis, angle) * rot_quat;
	rot_eul = math::RadToDeg(rot_quat.ToEulerXYZ());

	transform_modified = true;
}

void RE_CompTransform::GlobalRotate(math::vec axis, float angle)
{
	if (angle != 0.f)
	{
		math::float4x4 gm = GetGlobalMatrix();
		gm = gm.RotateAxisAngle(axis, angle);

		math::vec pos_global, scale_global;
		math::Quat desired_rot_global;

		if (go != nullptr)
		{
			
			gm.Decompose(pos_global, desired_rot_global, scale_global);
			rot_quat = desired_rot_global / go->GetParent()->GetTransform()->GetGlobalRot();

			//rot_quat = rot_quat.RotateAxisAngle(axis, angle) * rot_quat;
		}
		else
		{
			gm.Decompose(pos_global, desired_rot_global, scale_global);
			rot_quat = desired_rot_global * rot_quat;

		}

		rot_eul = math::RadToDeg(rot_quat.ToEulerXYZ());

		transform_modified = true;
	}
}

math::vec RE_CompTransform::GetGlobalPosition() const
{
	return (go != nullptr && go->GetParent() != nullptr) ? pos + go->GetParent()->GetTransform()->GetGlobalPosition() : pos;
}

math::Quat RE_CompTransform::GetGlobalRot() const
{
	return (go != nullptr && go->GetParent() != nullptr) ? rot_quat * go->GetParent()->GetTransform()->GetGlobalRot() : rot_quat;
}

math::vec RE_CompTransform::GetGlobalScale() const
{
	return (go != nullptr && go->GetParent() != nullptr) ? scale.Mul(go->GetParent()->GetTransform()->GetGlobalScale()) : scale;
}

math::vec RE_CompTransform::GetGlobalRight() const
{
	return right;
}

math::vec RE_CompTransform::GetGlobalUp() const
{
	return up;
}

math::vec RE_CompTransform::GetGlobalForward() const
{
	return front;
}

math::vec RE_CompTransform::GetLocalPosition() const
{
	return pos;
}

math::vec RE_CompTransform::GetLocalRotXYZ() const
{
	return rot_eul;
}

math::Quat RE_CompTransform::GetLocalRot() const
{
	return rot_quat;
}

math::vec RE_CompTransform::GetLocalScale() const
{
	return scale;
}

math::float4x4 RE_CompTransform::GetLocalMatrix() const
{
	return float4x4::FromTRS(pos, rot_quat, scale);
}

math::float4x4 RE_CompTransform::GetGlobalMatInvTrans() const
{
	return GetGlobalMatrix().InverseTransposed();
}

math::float4x4 RE_CompTransform::GetGlobalMatrix() const
{
	return float4x4::FromTRS(GetGlobalPosition(), GetGlobalRot(), GetGlobalScale());
}

void RE_CompTransform::LocalLookAt(math::vec& target_pos)
{
	math::vec direction = target_pos - pos;
	SetLocalRot(math::Quat::LookAt(front, direction.Normalized(), up, math::float3(0.f, 1.f, 0.f)) * rot_quat);
}

void RE_CompTransform::LocalMove(Dir dir, float speed)
{
	if (speed != 0.f)
	{
		transform_modified = true;

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

void RE_CompTransform::Orbit(float xoffset, float yoffset, math::vec& target)
{
	float distance = math::Distance3(math::float4(pos, 0.f), math::float4(target, 0.f));




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
		if (ImGui::DragFloat3("Position", p, 0.1f, -10000.f, 10000.f, "%.2f"))
			SetPos({ p[0], p[1], p[2] });

		// Rotation -----------------------------------------------------
		float r[3] = { rot_eul.x, rot_eul.y, rot_eul.z };
		if (ImGui::DragFloat3("Rotation", r, 0.1f, -360.f, 360.f, "%.2f"))
			SetLocalRot({ r[0], r[1], r[2] });

		// Scale -----------------------------------------------------
		float s[3] = { scale.x, scale.y, scale.z };
		if (ImGui::InputFloat3("Scale", s, 2))
			SetScale({ s[0], s[1], s[2] });
	}
}

void RE_CompTransform::Reset()
{
	SetPos(math::vec::zero);
	SetLocalRot(math::Quat::identity);
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
			math::float4x4 global_transform = parent->GetTransform()->GetGlobalMatrix().Inverted();

			right = global_transform.Col3(0).Normalized();
			up = global_transform.Col3(1).Normalized();
			front = global_transform.Col3(2).Normalized();

			for (auto child : go->GetChilds())
				child->GetTransform()->CalcGlobalTransform(call_transform_modified);
		}
		else
		{
			math::float4x4 global_transform =  GetLocalMatrix();

			right = global_transform.Col3(0).Normalized();
			up = global_transform.Col3(1).Normalized();
			front = global_transform.Col3(2).Normalized();

			for (auto child : go->GetChilds())
				child->GetTransform()->CalcGlobalTransform(call_transform_modified);
		}

		if (call_transform_modified) go->TransformModified();
	}
	else
	{
		math::float4x4 global_transform = GetLocalMatrix();

		right = global_transform.Col3(0).Normalized();
		up = global_transform.Col3(1).Normalized();
		front = global_transform.Col3(2).Normalized();
	}

	just_calculated_global = true;
}