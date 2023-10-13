#ifndef __RE_EMISSION_COLLIDER_H__
#define __RE_EMISSION_COLLIDER_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include "RE_EmissionSingleValue.h"

class RE_EmissionCollider : public RE_Serializable
{
public:

	enum class Type : ushort
	{
		NONE = 0,
		POINT,
		SPHERE
	};

public:

	Type type = Type(0);

	bool inter_collisions = false;

	RE_EmissionSingleValue mass = {};
	RE_EmissionSingleValue radius = {};
	RE_EmissionSingleValue restitution = {};

public:

	RE_EmissionCollider() = default;
	~RE_EmissionCollider() = default;

	bool DrawEditor();

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_EMISSION_COLLIDER_H__