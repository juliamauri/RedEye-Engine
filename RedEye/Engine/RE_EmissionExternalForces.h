#ifndef __RE_EMISSION_EXTERNAL_FORCES_H__
#define __RE_EMISSION_EXTERNAL_FORCES_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <MGL/Math/float3.h>

class RE_EmissionExternalForces : public RE_Serializable
{
public:

	enum class Type : ushort
	{
		NONE = 0,
		GRAVITY,
		WIND,
		WIND_GRAVITY
	};

public:

	Type type = Type(0);
	float gravity = -9.81f;
	math::vec wind = math::vec::zero;

public:

	RE_EmissionExternalForces() = default;
	~RE_EmissionExternalForces() = default;

	bool DrawEditor();

	math::vec GetAcceleration() const;

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_EMISSION_EXTERNAL_FORCES_H__