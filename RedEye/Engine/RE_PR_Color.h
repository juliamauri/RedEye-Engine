#ifndef __RE_PR_COLOR_H__
#define __RE_PR_COLOR_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"
#include "RE_Curve.h"

#include <MGL/Math/float3.h>

class RE_PR_Color : public RE_Serializable
{
public:

	enum class Type : ushort
	{
		SINGLE = 0,
		OVERLIFETIME,
		OVERDISTANCE,
		OVERSPEED
	};

public:

	Type type = Type(0);

	math::vec base = math::vec::one;
	math::vec gradient = math::vec::zero;

	bool useCurve = false;
	RE_Curve curve = {};

public:

	RE_PR_Color() = default;
	~RE_PR_Color() = default;

	bool DrawEditor();

	math::vec GetValue(const float weight = 1.f) const;

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_PR_COLOR_H__