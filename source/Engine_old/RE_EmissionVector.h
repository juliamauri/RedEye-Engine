#ifndef __RE_EMISSION_VECTOR_H__
#define __RE_EMISSION_VECTOR_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <MGL/Math/float3.h>

struct RE_EmissionVector : RE_Serializable
{
	RE_EmissionVector() = default;

	enum class Type : ushort
	{
		NONE = 0,
		VALUE,
		RANGEX,
		RANGEY,
		RANGEZ,
		RANGEXY,
		RANGEXZ,
		RANGEYZ,
		RANGEXYZ
	};

	Type type = Type(0);

	math::vec val = math::vec::zero;
	math::vec margin = math::vec::zero;

	math::vec GetValue() const;

	bool DrawEditor(const char* name);

	void JsonSerialize(RE_Json* node) const override;
	void JsonDeserialize(RE_Json* node) override;

	size_t GetBinarySize() const override;
	void BinarySerialize(char*& cursor) const override;
	void BinaryDeserialize(char*& cursor) override;
};

#endif // !__RE_EMISSION_VECTOR_H__