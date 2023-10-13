#ifndef __RE_PR_OPACITY_H__
#define __RE_PR_OPACITY_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"
#include "RE_Curve.h"

class RE_PR_Opacity : public RE_Serializable
{
public:

	enum class Type : ushort
	{
		NONE = 0,
		VALUE,
		OVERLIFETIME,
		OVERDISTANCE,
		OVERSPEED
	};

public:

	Type type = Type(0);

	float opacity = 1.0f;
	bool inverted = false;

	bool useCurve = false;
	RE_Curve curve = {};

public:

	RE_PR_Opacity() = default;
	~RE_PR_Opacity() = default;

	bool DrawEditor();

	float GetValue(const float weight) const;
	bool HasOpacity() const { return type != RE_PR_Opacity::Type::NONE; }

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_PR_OPACITY_H__