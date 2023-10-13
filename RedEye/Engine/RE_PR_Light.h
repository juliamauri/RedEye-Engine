#ifndef __RE_PR_LIGHT_H__
#define __RE_PR_LIGHT_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <MGL/Math/float3.h>

class RE_PR_Light : public RE_Serializable
{
public:

	enum class Type : ushort
	{
		NONE = 0,
		UNIQUE,
		PER_PARTICLE
	};

public:

	Type type = Type(0);

	bool random_color = false;
	bool random_i = false;
	bool random_s = false;

	math::vec color = math::vec::one;
	float intensity = 1.f;
	float specular = 0.2f;
	float intensity_max = 50.f;
	float specular_max = 1.f;

	float constant = 1.0f;
	float linear = 0.091f;
	float quadratic = 0.011f;

public:

	RE_PR_Light() = default;
	~RE_PR_Light() = default;

	bool DrawEditor(uint id);

	math::vec GetColor() const;
	float GetIntensity() const;
	float GetSpecular() const;
	math::vec GetQuadraticValues() const;
	bool HasLight() const { return type != Type::NONE; }

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_PR_LIGHT_H__