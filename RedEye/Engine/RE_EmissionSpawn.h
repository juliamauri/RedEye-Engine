#ifndef __RE_EMISSION_SPAWN_H__
#define __RE_EMISSION_SPAWN_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

struct RE_EmissionSpawn : RE_Serializable
{
	RE_EmissionSpawn() = default;

	enum class Type : ushort
	{
		SINGLE = 0,
		BURST,
		FLOW
	};

	Type type = Type(0);

	bool has_started = false;
	float time_offset = 0.f;
	int particles_spawned = 10;
	float frequency = 10.f;

	uint CountNewParticles(const float dt);

	bool DrawEditor(bool& changes);

	void JsonSerialize(RE_Json* node) const override;
	void JsonDeserialize(RE_Json* node) override;

	size_t GetBinarySize() const override;
	void BinarySerialize(char*& cursor) const override;
	void BinaryDeserialize(char*& cursor) override;
};

#endif // !__RE_EMISSION_SPAWN_H__