#include "RE_EmissionInterval.h"

#include "RE_Memory.h"
#include "RE_Json.h"
#include <ImGui/imgui.h>

bool RE_EmissionInterval::DrawEditor(bool& changes)
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Interval", &tmp, "None\0Intermitent\0Custom\0"))
	{
		type = static_cast<RE_EmissionInterval::Type>(tmp);
		is_open = true;
		time_offset = 0.f;
		duration[0] = duration[1] = 1.f;
		changes = ret = true;
	}

	switch (type)
	{
	case RE_EmissionInterval::Type::INTERMITENT:
	{
		if (ImGui::DragFloat("Interval On", &duration[1], 1.f, 0.f, 10000.f)) changes = true;
		break;
	}
	case RE_EmissionInterval::Type::CUSTOM:
	{
		if (ImGui::DragFloat("Interval On", &duration[1], 1.f, 0.f, 10000.f)) changes = true;
		if (ImGui::DragFloat("Interval Off", &duration[0], 1.f, 0.f, 10000.f)) changes = true;
		break;
	}
	default: break;
	}

	return ret;
}

bool RE_EmissionInterval::IsActive(float& dt)
{
	switch (type)
	{
	case Type::INTERMITENT:
	{
		if ((time_offset += dt) >= duration[1])
		{
			dt -= (time_offset -= duration[1]);
			is_open = !is_open;
		}

		break;
	}
	case Type::CUSTOM:
	{
		if ((time_offset += dt) >= duration[is_open])
		{
			dt -= (time_offset -= duration[is_open]);
			is_open = !is_open;
		}

		break;
	}
	default:
	{
		is_open = true;
		break;
	}
	}

	return is_open;
}

void RE_EmissionInterval::JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->Push("Type", static_cast<int>(type));
	if (type != Type::NONE)
	{
		node->Push("Duration 1", duration[0]);
		node->Push("Duration 2", duration[1]);
	}

	DEL(node)
}

void RE_EmissionInterval::JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources)
{
	type = static_cast<RE_EmissionInterval::Type>(node->PullInt("Type", static_cast<int>(type)));
	if (type != Type::NONE)
	{
		duration[0] = node->PullFloat("Duration 1", duration[0]);
		duration[1] = node->PullFloat("Duration 2", duration[1]);
	}

	DEL(node)
}

size_t RE_EmissionInterval::GetBinarySize() const
{
	size_t ret = sizeof(ushort);
	if (type != Type::NONE) ret += sizeof(float) * 2;
	return ret;
}

void RE_EmissionInterval::BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t  size = sizeof(ushort);
	memcpy(cursor, &type, size);
	cursor += size;

	if (type == Type::NONE) return;

	size = sizeof(float) * 2;
	memcpy(cursor, duration, size);
	cursor += size;
}

void RE_EmissionInterval::BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(ushort);
	memcpy(&type, cursor, size);
	cursor += size;

	if (type == Type::NONE) return;

	size = sizeof(float) * 2;
	memcpy(duration, cursor, size);
	cursor += size;
}
