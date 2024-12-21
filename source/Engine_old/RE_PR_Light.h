#ifndef __RE_PR_LIGHT_H__
#define __RE_PR_LIGHT_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <MGL/Math/float3.h>

struct RE_PR_Light : RE_Serializable
{
	RE_PR_Light() = default;

	enum class Type : ushort
	{
		NONE = 0,
		UNIQUE,
		PER_PARTICLE
	};

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

	bool HasLight() const;
	math::vec GetColor() const;
	float GetIntensity() const;
	float GetSpecular() const;
	math::vec GetQuadraticValues() const;

	bool DrawEditor(const unsigned int id);

	void JsonSerialize(RE_Json* node) const override;
	void JsonDeserialize(RE_Json* node) override;

	size_t GetBinarySize() const override;
	void BinarySerialize(char*& cursor) const override;
	void BinaryDeserialize(char*& cursor) override;
};

#endif // !__RE_PR_LIGHT_H__