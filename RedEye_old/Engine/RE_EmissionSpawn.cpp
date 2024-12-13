#include "RE_EmissionSpawn.h"

#include "RE_Memory.h"
#include "RE_Json.h"

#include <ImGui/imgui.h>

uint RE_EmissionSpawn::CountNewParticles(const float dt)
{
	uint ret = 0;

	time_offset += dt;

	switch (type)
	{
	case RE_EmissionSpawn::Type::SINGLE:
	{
		if (!has_started)
			ret = static_cast<unsigned int>(particles_spawned);

		break;
	}
	case RE_EmissionSpawn::Type::BURST:
	{
		auto mult = static_cast<int>(!has_started);
		while (time_offset >= frequency)
		{
			time_offset -= frequency;
			mult++;
		}

		ret += static_cast<unsigned int>(particles_spawned * mult);

		break;
	}
	case RE_EmissionSpawn::Type::FLOW:
	{
		ret = static_cast<unsigned int>(time_offset * frequency);
		time_offset -= static_cast<float>(ret) / frequency;
		break;
	}
	}

	has_started = true;

	return ret;
}

bool RE_EmissionSpawn::DrawEditor(bool& changes)
{
	bool ret = false;
	int tmp = static_cast<int>(type);
	if (ImGui::Combo("Spawn type", &tmp, "Single\0Burst\0Flow\0"))
	{
		type = static_cast<RE_EmissionSpawn::Type>(tmp);
		has_started = false;
		time_offset = 0.f;
		changes = ret = true;
	}

	switch (type)
	{
	case RE_EmissionSpawn::Type::SINGLE:
		if (ImGui::DragInt("Particle amount", &particles_spawned, 1.f, 0, 10000)) changes = true;
		break;
	case RE_EmissionSpawn::Type::BURST:
		if (ImGui::DragInt("Particles/burst", &particles_spawned, 1.f, 0, 10000) ||
			ImGui::DragFloat("Period", &frequency, 1.f, 0.0001f, 10000.f)) changes = true;
		break;
	case RE_EmissionSpawn::Type::FLOW:
		if (ImGui::DragFloat("Frecuency", &frequency, 1.f, 0.0001f, 1000.f)) changes = true;
		break;
	default: break;
	}

	return ret;
}

void RE_EmissionSpawn::JsonSerialize(RE_Json* node) const
{
	node->Push("Type", static_cast<uint>(type));
	switch (type)
	{
	case RE_EmissionSpawn::Type::SINGLE:
		node->Push("Particles spawned", particles_spawned);
		break;
	case RE_EmissionSpawn::Type::BURST:
		node->Push("Particles spawned", particles_spawned);
		node->Push("Period", frequency);
		break;
	case RE_EmissionSpawn::Type::FLOW:
		node->Push("Frequency", frequency);
		break;
	default: break;
	}

	DEL(node)
}

void RE_EmissionSpawn::JsonDeserialize(RE_Json* node)
{
	type = static_cast<RE_EmissionSpawn::Type>(node->PullUInt("Type", static_cast<uint>(type)));
	switch (type) {
	case RE_EmissionSpawn::Type::SINGLE:
		particles_spawned = node->PullInt("Particles spawned", particles_spawned);
		break;
	case RE_EmissionSpawn::Type::BURST:
		particles_spawned = node->PullInt("Particles spawned", particles_spawned);
		frequency = node->PullFloat("Period", frequency);
		break;
	case RE_EmissionSpawn::Type::FLOW:
		frequency = node->PullFloat("Frequency", frequency);
		break;
	}

	DEL(node)
}

size_t RE_EmissionSpawn::GetBinarySize() const
{
	size_t ret = sizeof(ushort);
	switch (type)
	{
	case RE_EmissionSpawn::Type::SINGLE: ret *= 2; break;
	case RE_EmissionSpawn::Type::BURST: ret *= 2; ret += sizeof(float); break;
	case RE_EmissionSpawn::Type::FLOW: ret += sizeof(float); break;
	default: break;
	}
	return ret;
}

void RE_EmissionSpawn::BinarySerialize(char*& cursor) const
{
	size_t size = sizeof(ushort);
	memcpy(cursor, &type, size);
	cursor += size;
	switch (type)
	{
	case RE_EmissionSpawn::Type::SINGLE:
		memcpy(cursor, &particles_spawned, size);
		cursor += size;
		break;
	case RE_EmissionSpawn::Type::BURST:
		memcpy(cursor, &particles_spawned, size);
		cursor += size;
		size = sizeof(float);
		memcpy(cursor, &frequency, size);
		cursor += size;
		break;
	case RE_EmissionSpawn::Type::FLOW:
		size = sizeof(float);
		memcpy(cursor, &frequency, size);
		cursor += size;
		break;
	default: break;
	}
}

void RE_EmissionSpawn::BinaryDeserialize(char*& cursor)
{
	size_t size = sizeof(ushort);
	memcpy(&type, cursor, size);
	cursor += size;
	switch (type) {
	case RE_EmissionSpawn::Type::SINGLE:
	{
		memcpy(&particles_spawned, cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionSpawn::Type::BURST:
	{
		memcpy(&particles_spawned, cursor, size);
		cursor += size;
		size = sizeof(float);
		memcpy(&frequency, cursor, size);
		cursor += size;
		break;
	}
	case RE_EmissionSpawn::Type::FLOW:
	{
		size = sizeof(float);
		memcpy(&frequency, cursor, size);
		cursor += size;
		break;
	}
	}
}