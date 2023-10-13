#ifndef __RE_EMISSION_VECTOR_H__
#define __RE_EMISSION_VECTOR_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <MGL/Math/float3.h>

class RE_EmissionVector : public RE_Serializable
{
public:

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

public:

	Type type = Type(0);
	math::vec val = math::vec::zero;
	math::vec margin = math::vec::zero;

public:

	RE_EmissionVector() = default;
	~RE_EmissionVector() = default;

	bool DrawEditor(const char* name);

	math::vec GetValue() const;

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_EMISSION_VECTOR_H__