#ifndef __RE_EMISSION_INTERVAL_H__
#define __RE_EMISSION_INTERVAL_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

struct RE_EmissionInterval : RE_Serializable
{
	RE_EmissionInterval() = default;

	enum class Type : ushort
	{
		NONE = 0,
		INTERMITENT,
		CUSTOM
	};

	Type type = Type(0);

	bool is_open = false;
	float time_offset = 0.f;
	float duration[2] = { 1.f, 1.f };

	bool IsActive(float& dt);

	bool DrawEditor(bool& changes);

	void JsonSerialize(RE_Json* node) const override;
	void JsonDeserialize(RE_Json* node) override;

	size_t GetBinarySize() const override;
	void BinarySerialize(char*& cursor) const override;
	void BinaryDeserialize(char*& cursor) override;
};

#endif // !__RE_EMISSION_INTERVAL_H__