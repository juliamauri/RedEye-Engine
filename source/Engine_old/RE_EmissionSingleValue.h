#ifndef __RE_EMISSION_SINGLE_VALUE_H__
#define __RE_EMISSION_SINGLE_VALUE_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

struct RE_EmissionSingleValue : RE_Serializable
{
	RE_EmissionSingleValue() = default;

	enum class Type : ushort
	{
		NONE = 0,
		VALUE,
		RANGE
	};

	Type type = Type(0);

	float val = 1.f;
	float margin = 1.f;

	float GetValue() const;
	float GetMin() const;
	float GetMax() const;

	bool DrawEditor(const char* name);

	void JsonSerialize(RE_Json* node) const override;
	void JsonDeserialize(RE_Json* node) override;

	size_t GetBinarySize() const override;
	void BinarySerialize(char*& cursor) const override;
	void BinaryDeserialize(char*& cursor) override;
};

#endif // !__RE_EMISSION_SINGLE_VALUE_H__