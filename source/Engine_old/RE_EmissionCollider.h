#ifndef __RE_EMISSION_COLLIDER_H__
#define __RE_EMISSION_COLLIDER_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include "RE_EmissionSingleValue.h"

struct RE_EmissionCollider : RE_Serializable
{
	RE_EmissionCollider() = default;

	enum class Type : ushort
	{
		NONE = 0,
		POINT,
		SPHERE
	};

	Type type = Type(0);

	bool inter_collisions = false;

	RE_EmissionSingleValue mass = {};
	RE_EmissionSingleValue radius = {};
	RE_EmissionSingleValue restitution = {};

	bool DrawEditor();

	void JsonSerialize(RE_Json* node) const override;
	void JsonDeserialize(RE_Json* node) override;

	size_t GetBinarySize() const override;
	void BinarySerialize(char*& cursor) const override;
	void BinaryDeserialize(char*& cursor) override;
};

#endif // !__RE_EMISSION_COLLIDER_H__