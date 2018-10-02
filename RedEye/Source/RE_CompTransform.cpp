#include "RE_CompTransform.h"

#include "RE_GameObject.h"

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
		local_transform = local_transform.FromTRS(pos, rot_quat, scale);
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
	rot_quat = rot_quat.FromEulerXYZ(rot_deg.x, rot_deg.y, rot_deg.z);
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

void RE_CompTransform::DrawProperties()
{
	// Draw in properties
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

		if (call_tranf_modified) go->TransformModified();
	}
}
