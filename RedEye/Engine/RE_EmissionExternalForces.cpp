#include "RE_EmissionExternalForces.h"

#include "RE_Memory.h"
#include "RE_Json.h"

#include <ImGui/imgui.h>

math::vec RE_EmissionExternalForces::GetAcceleration() const
{
	math::vec ret = math::vec::zero;

	switch (type)
	{
	case Type::GRAVITY: ret = math::vec(0.f, gravity, 0.f);
	case Type::WIND: ret = wind;
	case Type::WIND_GRAVITY: ret = math::vec(wind.x, wind.y + gravity, wind.z);
	default: break;
	}

	return ret;
}

bool RE_EmissionExternalForces::DrawEditor()
{
	bool ret = false;

	int next_type = static_cast<int>(type);
	if (ImGui::Combo("External Forces", &next_type, "None\0Gravity\0Wind\0Gravity + Wind\0"))
	{
		type = static_cast<Type>(next_type);
		ret = true;
	}

	switch (type)
	{
	case Type::GRAVITY:
		if (ImGui::DragFloat("Gravity", &gravity))
			ret = true;
		break;
	case Type::WIND:
		if (ImGui::DragFloat3("Wind", wind.ptr()))
			ret = true;
		break;
	case Type::WIND_GRAVITY:
		if (ImGui::DragFloat("Gravity", &gravity) ||
			ImGui::DragFloat3("Wind", wind.ptr()))
			ret = true;
		break;
	default: break;
	}

	return ret;
}

void RE_EmissionExternalForces::JsonSerialize(RE_Json* node) const
{
	node->Push("Type", static_cast<uint>(type));

	switch (type)
	{
	case Type::GRAVITY:
		node->Push("Gravity", gravity);
		break;
	case Type::WIND:
		node->PushFloatVector("Wind", wind);
		break;
	case Type::WIND_GRAVITY:
		node->Push("Gravity", gravity);
		node->PushFloatVector("Wind", wind);
		break;
	default: break;
	}

	DEL(node);
}

void RE_EmissionExternalForces::JsonDeserialize(RE_Json* node)
{
	type = static_cast<Type>(node->PullUInt("Type", static_cast<uint>(type)));

	switch (type)
	{
	case Type::GRAVITY:
		gravity = node->PullFloat("Gravity", gravity);
		break;
	case Type::WIND:
		wind = node->PullFloatVector("Wind", wind);
		break;
	case Type::WIND_GRAVITY:
		gravity = node->PullFloat("Gravity", gravity);
		wind = node->PullFloatVector("Wind", wind);
		break;
	default: break;
	}

	DEL(node);
}

size_t RE_EmissionExternalForces::GetBinarySize() const
{
	size_t ret = sizeof(ushort);
	switch (type)
	{
	case RE_EmissionExternalForces::Type::GRAVITY: ret += sizeof(float); break;
	case RE_EmissionExternalForces::Type::WIND: ret += sizeof(float) * 3; break;
	case RE_EmissionExternalForces::Type::WIND_GRAVITY: ret += sizeof(float) * 4; break;
	default: break;
	}
	return ret;
}

void RE_EmissionExternalForces::BinarySerialize(char*& cursor) const
{
	size_t size = sizeof(ushort);
	memcpy(cursor, &type, size);
	cursor += size;

	switch (type)
	{
	case Type::GRAVITY:
		size = sizeof(float);
		memcpy(cursor, &gravity, size);
		cursor += size;
		break;
	case Type::WIND:
		size = sizeof(float) * 3;
		memcpy(cursor, wind.ptr(), size);
		cursor += size;
		break;
	case Type::WIND_GRAVITY:
		size = sizeof(float);
		memcpy(cursor, &gravity, size);
		cursor += size;
		size *= 3;
		memcpy(cursor, wind.ptr(), size);
		cursor += size;
		break;
	default: break;
	}
}

void RE_EmissionExternalForces::BinaryDeserialize(char*& cursor)
{
	size_t size = sizeof(int);
	memcpy(&type, cursor, size);
	cursor += size;

	switch (type)
	{
	case Type::GRAVITY:
		size = sizeof(float);
		memcpy(&gravity, cursor, size);
		cursor += size;
		break;
	case Type::WIND:
		size = sizeof(float) * 3;
		memcpy(wind.ptr(), cursor, size);
		cursor += size;
		break;
	case Type::WIND_GRAVITY:
		size = sizeof(float);
		memcpy(&gravity, cursor, size);
		cursor += size;
		size *= 3u;
		memcpy(wind.ptr(), cursor, size);
		cursor += size;
		break;
	default: break;
	}
}