#include "RE_EmissionVector.h"

#include "RE_Memory.h"
#include "Application.h"
#include "RE_Math.h"
#include "RE_Json.h"

#include <ImGui/imgui.h>

math::vec RE_EmissionVector::GetValue() const
{
	math::vec ret = math::vec::zero;
	switch (type)
	{
	case Type::VALUE: ret = val;
	case Type::RANGEX: ret = { val.x + (RE_MATH->RandomFN() * margin.x), val.y, val.z };
	case Type::RANGEY: ret = { val.x, val.y + (RE_MATH->RandomFN() * margin.y), val.z };
	case Type::RANGEZ: ret = { val.x, val.y, val.z + (RE_MATH->RandomFN() * margin.z) };
	case Type::RANGEXY: ret = { val.x + (RE_MATH->RandomFN() * margin.x), val.y + (RE_MATH->RandomFN() * margin.y), val.z };
	case Type::RANGEXZ: ret = { val.x + (RE_MATH->RandomFN() * margin.x), val.y, val.z + (RE_MATH->RandomFN() * margin.z) };
	case Type::RANGEYZ: ret = { val.x, val.y + (RE_MATH->RandomFN() * margin.y), val.z + (RE_MATH->RandomFN() * margin.z) };
	case Type::RANGEXYZ: ret = { val.x + (RE_MATH->RandomFN() * margin.x), val.y + (RE_MATH->RandomFN() * margin.y), val.z + (RE_MATH->RandomFN() * margin.z) };
	default: break;
	}
	return math::vec::zero;
}

bool RE_EmissionVector::DrawEditor(const char* name)
{
	bool ret = false;
	const eastl::string tmp(name);
	int next_type = static_cast<int>(type);
	if (ImGui::Combo((tmp + " type").c_str(), &next_type, "None\0Value\0Range X\0Range Y\0Range Z\0Range XY\0Range XZ\0Range YZ\0Range XYZ\0"))
	{
		type = static_cast<Type>(next_type);
		ret = true;
	}

	switch (type)
	{
	case Type::VALUE:
		ImGui::DragFloat3(name, val.ptr()); break;
	case Type::RANGEX:
		if (ImGui::DragFloat3(name, val.ptr()) ||
			ImGui::DragFloat((tmp + " X Margin").c_str(), &margin.x))
			ret = true;
		break;
	case Type::RANGEY:
		if (ImGui::DragFloat3(name, val.ptr()) ||
			ImGui::DragFloat((tmp + " Y Margin").c_str(), &margin.y))
			ret = true;
		break;
	case Type::RANGEZ:
		if (ImGui::DragFloat3(name, val.ptr()) ||
			ImGui::DragFloat((tmp + " Z Margin").c_str(), &margin.z))
			ret = true;
		break;
	case Type::RANGEXY:
		if (ImGui::DragFloat3(name, val.ptr()) ||
			ImGui::DragFloat2((tmp + " XY Margin").c_str(), &margin.x) ||
			ImGui::DragFloat((tmp + " Y Margin").c_str(), &margin.y))
			ret = true;
		break;
	case Type::RANGEXZ:
	{
		if (ImGui::DragFloat3(name, val.ptr())) ret = true;
		float xz[2] = { margin.x, margin.z };
		if (ImGui::DragFloat2((tmp + " XZ Margin").c_str(), xz))
		{
			margin.x = xz[0];
			margin.z = xz[1];
			ret = true;
		}
		break;
	}
	case Type::RANGEYZ:
		if (ImGui::DragFloat3(name, val.ptr()) ||
			ImGui::DragFloat2((tmp + " YZ Margin").c_str(), margin.ptr() + 1))
			ret = true;
		break;
	case Type::RANGEXYZ:
		if (ImGui::DragFloat3(name, val.ptr()) ||
			ImGui::DragFloat3((tmp + " Margin").c_str(), margin.ptr())) ret = true;
		break;
	default: break;
	}

	return ret;
}

void RE_EmissionVector::JsonSerialize(RE_Json* node) const
{
	node->Push("Type", static_cast<uint>(type));

	switch (type)
	{
	case RE_EmissionVector::Type::VALUE:
		node->PushFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEX:
		node->Push("Margin X", margin.x);
		node->PushFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEY:
		node->Push("Margin Y", margin.y);
		node->PushFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEZ:
		node->Push("Margin Z", margin.z);
		node->PushFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEXY:
		node->Push("Margin X", margin.x);
		node->Push("Margin Y", margin.y);
		node->PushFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEXZ:
		node->Push("Margin X", margin.x);
		node->Push("Margin Z", margin.z);
		node->PushFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEYZ:
		node->Push("Margin Y", margin.y);
		node->Push("Margin Z", margin.z);
		node->PushFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEXYZ:
		node->PushFloatVector("Margin", margin);
		node->PushFloatVector("Value", val);
		break;
	default: break;
	}

	DEL(node);
}

void RE_EmissionVector::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_EmissionVector::Type>(node->PullUInt("Type", static_cast<uint>(type)));

	switch (type)
	{
	case RE_EmissionVector::Type::VALUE:
		val = node->PullFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEX:
		margin.x = node->PullFloat("Margin X", margin.x);
		val = node->PullFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEY:
		margin.y = node->PullFloat("Margin Y", margin.y);
		val = node->PullFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEZ:
		margin.z = node->PullFloat("Margin Z", margin.z);
		val = node->PullFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEXY:
		margin.x = node->PullFloat("Margin X", margin.x);
		margin.y = node->PullFloat("Margin Y", margin.y);
		val = node->PullFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEXZ:
		margin.x = node->PullFloat("Margin X", margin.x);
		margin.z = node->PullFloat("Margin Z", margin.z);
		val = node->PullFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEYZ:
		margin.y = node->PullFloat("Margin Y", margin.y);
		margin.z = node->PullFloat("Margin Z", margin.z);
		val = node->PullFloatVector("Value", val);
		break;
	case RE_EmissionVector::Type::RANGEXYZ:
		margin = node->PullFloatVector("Margin", margin);
		val = node->PullFloatVector("Value", val);
		break;
	default: break;
	}

	DEL(node);
}

size_t RE_EmissionVector::GetBinarySize() const
{
	size_t ret = sizeof(float);

	switch (type)
	{
	case RE_EmissionVector::Type::VALUE: ret *= 3; break;
	case RE_EmissionVector::Type::RANGEX: ret *= 4; break;
	case RE_EmissionVector::Type::RANGEY: ret *= 4; break;
	case RE_EmissionVector::Type::RANGEZ: ret *= 4; break;
	case RE_EmissionVector::Type::RANGEXY: ret *= 5; break;
	case RE_EmissionVector::Type::RANGEXZ: ret *= 5; break;
	case RE_EmissionVector::Type::RANGEYZ: ret *= 5; break;
	case RE_EmissionVector::Type::RANGEXYZ: ret *= 6; break;
	default: break;
	}

	ret += sizeof(ushort);
	return ret;
}

void RE_EmissionVector::BinarySerialize(char*& cursor) const
{
	size_t size = sizeof(ushort);
	memcpy(cursor, &type, size);
	cursor += size;
	switch (type) {
	case RE_EmissionVector::Type::VALUE:
		size = sizeof(float) * 3;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEX:
		size = sizeof(float);
		memcpy(cursor, &margin.x, size);
		cursor += size;
		size *= 3;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEY:
		size = sizeof(float);
		memcpy(cursor, &margin.y, size);
		cursor += size;
		size *= 3;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEZ:
		size = sizeof(float);
		memcpy(cursor, &margin.z, size);
		cursor += size;
		size *= 3;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEXY:
		size = sizeof(float);
		memcpy(cursor, &margin.x, size);
		cursor += size;
		memcpy(cursor, &margin.y, size);
		cursor += size;
		size *= 3;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEXZ:
		size = sizeof(float);
		memcpy(cursor, &margin.x, size);
		cursor += size;
		memcpy(cursor, &margin.z, size);
		cursor += size;
		size *= 3;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEYZ:
		size = sizeof(float);
		memcpy(cursor, &margin.y, size);
		cursor += size;
		memcpy(cursor, &margin.z, size);
		cursor += size;
		size *= 3;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEXYZ:
		size = sizeof(float) * 3;
		memcpy(cursor, margin.ptr(), size);
		cursor += size;
		memcpy(cursor, val.ptr(), size);
		cursor += size;
		break;
	default: break;
	}
}

void RE_EmissionVector::BinaryDeserialize(char*& cursor)
{
	size_t size = sizeof(ushort);
	memcpy(&type, cursor, size);
	cursor += size;

	switch (type)
	{
	case RE_EmissionVector::Type::VALUE:
		size = sizeof(float) * 3;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEX:
		size = sizeof(float);
		memcpy(&margin.x, cursor, size);
		cursor += size;
		size *= 3;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEY:
		size = sizeof(float);
		memcpy(&margin.y, cursor, size);
		cursor += size;
		size *= 3;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEZ:
		size = sizeof(float);
		memcpy(&margin.z, cursor, size);
		cursor += size;
		size *= 3;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEXY:
		size = sizeof(float);
		memcpy(&margin.x, cursor, size);
		cursor += size;
		memcpy(&margin.y, cursor, size);
		cursor += size;
		size *= 3;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEXZ:
		size = sizeof(float);
		memcpy(&margin.x, cursor, size);
		cursor += size;
		memcpy(&margin.z, cursor, size);
		cursor += size;
		size *= 3;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEYZ:
		size = sizeof(float);
		memcpy(&margin.y, cursor, size);
		cursor += size;
		memcpy(&margin.z, cursor, size);
		cursor += size;
		size *= 3;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	case RE_EmissionVector::Type::RANGEXYZ:
		size = sizeof(float) * 3;
		memcpy(margin.ptr(), cursor, size);
		cursor += size;
		memcpy(val.ptr(), cursor, size);
		cursor += size;
		break;
	default: break;
	}
}
