#include "RE_EmissionSingleValue.h"

#include "RE_Memory.h"
#include "Application.h"
#include "RE_Random.h"
#include "RE_Json.h"

#include <ImGui/imgui.h>


float RE_EmissionSingleValue::GetValue() const
{
	float ret = 0.f;

	switch (type)
	{
	case Type::VALUE: ret = val;
	case Type::RANGE: ret = val + (RE_Random::RandomFN() * margin);
	default: break;
	}

	return ret;
}

float RE_EmissionSingleValue::GetMin() const
{
	float ret = 0.f;

	switch (type)
	{
	case Type::VALUE: ret = val;
	case Type::RANGE: ret = val - margin;
	default: break;
	}

	return ret;
}

float RE_EmissionSingleValue::GetMax() const
{
	float ret = 0.f;

	switch (type)
	{
	case Type::VALUE: ret = val;
	case Type::RANGE: ret = val + margin;
	default: break;
	}

	return ret;
}

bool RE_EmissionSingleValue::DrawEditor(const char* name)
{
	bool ret = false;

	const eastl::string tmp(name);
	int next_type = static_cast<int>(type);
	if (ImGui::Combo((tmp + " type").c_str(), &next_type, "None\0Value\0Range\0"))
	{
		type = static_cast<Type>(next_type);
		ret = true;
	}

	switch (type)
	{
	case Type::VALUE:
		if (ImGui::DragFloat(name, &val))
			ret = true;
		break;
	case Type::RANGE:
		if (ImGui::DragFloat(name, &val) ||
			ImGui::DragFloat((tmp + " Margin").c_str(), &margin, 0.01f, 0.f))
			ret = true;
		break;
	default: break;
	}

	return ret;
}

void RE_EmissionSingleValue::JsonSerialize(RE_Json* node) const
{
	node->Push("Type", static_cast<uint>(type));

	switch (type)
	{
	case RE_EmissionSingleValue::Type::VALUE:
	{
		node->Push("Value", val);
		break;
	}
	case RE_EmissionSingleValue::Type::RANGE:
	{
		node->Push("Value", val);
		node->Push("Margin", margin);
		break;
	}
	default: break;
	}

	DEL(node)
}

void RE_EmissionSingleValue::JsonDeserialize(RE_Json* node)
{
	type = static_cast<Type>(node->PullUInt("Type", static_cast<uint>(type)));

	switch (type)
	{
	case RE_EmissionSingleValue::Type::VALUE:
		val = node->PullFloat("Value", val);
		break;
	case RE_EmissionSingleValue::Type::RANGE:
		val = node->PullFloat("Value", val);
		margin = node->PullFloat("Margin", margin);
		break;
	default: break;
	}

	DEL(node)
}

size_t RE_EmissionSingleValue::GetBinarySize() const
{
	size_t ret = sizeof(ushort);
	switch (type)
	{
	case RE_EmissionSingleValue::Type::VALUE: ret += sizeof(float); break;
	case RE_EmissionSingleValue::Type::RANGE: ret += sizeof(float) * 2; break;
	default: break;
	}
	return ret;
}

void RE_EmissionSingleValue::BinarySerialize(char*& cursor) const
{
	size_t size = sizeof(ushort);
	memcpy(cursor, &type, size);
	cursor += size;

	switch (type)
	{
	case RE_EmissionSingleValue::Type::VALUE:
		size = sizeof(float);
		memcpy(cursor, &val, size);
		cursor += size;
		break;
	case RE_EmissionSingleValue::Type::RANGE:
		size = sizeof(float);
		memcpy(cursor, &val, size);
		cursor += size;
		memcpy(cursor, &margin, size);
		cursor += size;
		break;
	default: break;
	}
}

void RE_EmissionSingleValue::BinaryDeserialize(char*& cursor)
{
	size_t size = sizeof(ushort);
	memcpy(&type, cursor, size);
	cursor += size;

	switch (type)
	{
	case RE_EmissionSingleValue::Type::VALUE:
		size = sizeof(float);
		memcpy(&val, cursor, size);
		cursor += size;
		break;
	case RE_EmissionSingleValue::Type::RANGE:
		size = sizeof(float);
		memcpy(&val, cursor, size);
		cursor += size;
		memcpy(&margin, cursor, size);
		cursor += size;
		break;
	default: break;
	}
}