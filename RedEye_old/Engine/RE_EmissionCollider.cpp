#include "RE_EmissionCollider.h"

#include "RE_Memory.h"
#include "RE_Json.h"

#include <ImGui/imgui.h>

bool RE_EmissionCollider::DrawEditor()
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Collider Type", &tmp, "None\0Point\0Sphere\0"))
	{
		type = static_cast<Type>(tmp);
		ret = true;
	}

	if (type != Type::NONE &&
		ImGui::Checkbox("Inter collisions", &inter_collisions) ||
		mass.DrawEditor("Mass") ||
		restitution.DrawEditor("Restitution") ||
		(type == Type::SPHERE && radius.DrawEditor("Collider Radius")))
		ret = true;

	return ret;
}

void RE_EmissionCollider::JsonSerialize(RE_Json* node) const
{
	node->Push("Type", static_cast<uint>(type));

	if (type != Type::NONE)
	{
		node->Push("Inter collisions", inter_collisions);

		switch (type)
		{
		case Type::POINT:
			mass.JsonSerialize(node->PushJObject("Mass"));
			restitution.JsonSerialize(node->PushJObject("Restitution"));
			break;
		case Type::SPHERE:
			mass.JsonSerialize(node->PushJObject("Mass"));
			radius.JsonSerialize(node->PushJObject("Radius"));
			restitution.JsonSerialize(node->PushJObject("Restitution"));
			break;
		default: break;
		}
	}

	DEL(node)
}

void RE_EmissionCollider::JsonDeserialize(RE_Json* node)
{
	type = static_cast<Type>(node->PullUInt("Type", static_cast<uint>(type)));

	if (type != Type::NONE)
	{
		inter_collisions = node->PullBool("Inter collisions", inter_collisions);

		switch (type)
		{
		case Type::POINT:
			mass.JsonDeserialize(node->PullJObject("Mass"));
			restitution.JsonDeserialize(node->PullJObject("Restitution"));
			break;
		case Type::SPHERE:
			mass.JsonDeserialize(node->PullJObject("Mass"));
			radius.JsonDeserialize(node->PullJObject("Radius"));
			restitution.JsonDeserialize(node->PullJObject("Restitution"));
			break;
		default: break;
		}
	}

	DEL(node)
}

size_t RE_EmissionCollider::GetBinarySize() const
{
	size_t ret = sizeof(ushort);

	if (type != Type::NONE)
	{
		ret += sizeof(bool);
		switch (type)
		{
		case RE_EmissionCollider::Type::POINT:
			ret += mass.GetBinarySize();
			ret += restitution.GetBinarySize();
			break;
		case RE_EmissionCollider::Type::SPHERE:
			ret += mass.GetBinarySize();
			ret += radius.GetBinarySize();
			ret += restitution.GetBinarySize();
			break;
		default: break;
		}
	}

	return ret;
}

void RE_EmissionCollider::BinarySerialize(char*& cursor) const
{
	size_t size = sizeof(ushort);
	memcpy(cursor, &type, size);
	cursor += size;

	if (type != Type::NONE)
	{
		size = sizeof(bool);
		memcpy(cursor, &inter_collisions, size);
		cursor += size;

		switch (type)
		{
		case Type::POINT:
			mass.BinarySerialize(cursor);
			restitution.BinarySerialize(cursor);
			break;
		case Type::SPHERE:
			mass.BinarySerialize(cursor);
			radius.BinarySerialize(cursor);
			restitution.BinarySerialize(cursor);
			break;
		default: break;
		}
	}
}

void RE_EmissionCollider::BinaryDeserialize(char*& cursor)
{
	size_t size = sizeof(ushort);
	memcpy(&type, cursor, size);
	cursor += size;

	if (type != Type::NONE)
	{
		size = sizeof(bool);
		memcpy(&inter_collisions, cursor, size);
		cursor += size;

		switch (type)
		{
		case Type::POINT:
			mass.BinaryDeserialize(cursor);
			restitution.BinaryDeserialize(cursor);
			break;
		case Type::SPHERE:
			mass.BinaryDeserialize(cursor);
			radius.BinaryDeserialize(cursor);
			restitution.BinaryDeserialize(cursor);
			break;
		default: break;
		}
	}
}
