#ifndef __RE_PR_COLOR_H__
#define __RE_PR_COLOR_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"
#include "RE_Curve.h"

#include <MGL/Math/float3.h>

struct RE_PR_Color : RE_Serializable
{
	RE_PR_Color() = default;

	enum class Type : ushort
	{
		SINGLE = 0,
		OVERLIFETIME,
		OVERDISTANCE,
		OVERSPEED
	};

	Type type = Type(0);

	math::vec base = math::vec::one;
	math::vec gradient = math::vec::zero;

	bool useCurve = false;
	RE_Curve curve = {};

	math::vec GetValue(const float weight = 1.f) const;

	bool DrawEditor();

	void JsonSerialize(RE_Json* node) const override;
	void JsonDeserialize(RE_Json* node) override;

	size_t GetBinarySize() const override;
	void BinarySerialize(char*& cursor) const override;
	void BinaryDeserialize(char*& cursor) override;
};

#endif // !__RE_PR_COLOR_H__