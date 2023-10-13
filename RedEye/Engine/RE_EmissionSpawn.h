#ifndef __RE_EMISSION_SPAWN_H__
#define __RE_EMISSION_SPAWN_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

class RE_EmissionSpawn : public RE_Serializable
{
public:

	enum class Type : ushort
	{
		SINGLE = 0,
		BURST,
		FLOW
	};

public:

	Type type = Type(0);

	bool has_started = false;
	float time_offset = 0.f;
	int particles_spawned = 10;
	float frequency = 10.f;

public:

	RE_EmissionSpawn() = default;
	~RE_EmissionSpawn() = default;

	bool DrawEditor(bool& changes);
	uint CountNewParticles(const float dt);

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_EMISSION_SPAWN_H__