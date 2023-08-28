#ifndef __RE_PR_OPACITY_H__
#define __RE_PR_OPACITY_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"
#include "RE_Curve.h"

struct RE_PR_Opacity : RE_Serializable
{
	RE_PR_Opacity() = default;

	enum class Type : ushort
	{
		NONE = 0,
		VALUE,
		OVERLIFETIME,
		OVERDISTANCE,
		OVERSPEED
	};

	Type type = Type(0);

	float opacity = 1.0f;
	bool inverted = false;

	bool useCurve = false;
	RE_Curve curve = {};

	float GetValue(const float weight) const;

	bool DrawEditor();

	void JsonSerialize(RE_Json* node) const override;
	void JsonDeserialize(RE_Json* node) override;

	size_t GetBinarySize() const override;
	void BinarySerialize(char*& cursor) const override;
	void BinaryDeserialize(char*& cursor) override;
};

#endif // !__RE_PR_OPACITY_H__