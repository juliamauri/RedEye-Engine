#ifndef __RE_EMISSION_SINGLE_VALUE_H__
#define __RE_EMISSION_SINGLE_VALUE_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

class RE_EmissionSingleValue : public RE_Serializable
{
public:

	enum class Type : ushort
	{
		NONE = 0,
		VALUE,
		RANGE
	};

public:

	Type type = Type(0);

	float val = 1.f;
	float margin = 1.f;

public:

	RE_EmissionSingleValue() = default;
	~RE_EmissionSingleValue() = default;

	float GetValue() const;
	float GetMin() const;
	float GetMax() const;

	bool DrawEditor(const char* name);

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_EMISSION_SINGLE_VALUE_H__