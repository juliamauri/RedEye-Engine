#ifndef __RE_EMISSION_EXTERNAL_FORCES_H__
#define __RE_EMISSION_EXTERNAL_FORCES_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <MGL/Math/float3.h>

struct RE_EmissionExternalForces : RE_Serializable
{
	RE_EmissionExternalForces() = default;

	enum class Type : ushort
	{
		NONE = 0,
		GRAVITY,
		WIND,
		WIND_GRAVITY
	};

	Type type = Type(0);

	float gravity = -9.81f;
	math::vec wind = math::vec::zero;

	math::vec GetAcceleration() const;

	bool DrawEditor();

	void JsonSerialize(RE_Json* node) const override;
	void JsonDeserialize(RE_Json* node) override;

	size_t GetBinarySize() const override;
	void BinarySerialize(char*& cursor) const override;
	void BinaryDeserialize(char*& cursor) override;
};

#endif // !__RE_EMISSION_EXTERNAL_FORCES_H__