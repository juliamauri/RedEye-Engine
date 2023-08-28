#ifndef __RE_EMISSION_SHAPE_H__
#define __RE_EMISSION_SHAPE_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <MGL/Geometry/AABB.h>
#include <MGL/Geometry/Circle.h>
#include <MGL/Geometry/Sphere.h>
#include <EASTL/vector.h>

struct RE_EmissionShape : RE_Serializable
{
	RE_EmissionShape() = default;

	enum class Type : ushort
	{
		POINT = 0,
		CIRCLE,
		RING,
		AABB,
		SPHERE,
		HOLLOW_SPHERE
	};

	Type type = Type(0);

	union Geo
	{
		math::vec point;
		math::Circle circle = math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f);
		eastl::pair<math::Circle, float> ring;
		math::AABB box;
		math::Sphere sphere;
		eastl::pair<math::Sphere, float> hollow_sphere;
	};

	Geo geo = {};

	bool IsShaped() const;
	math::vec GetPosition() const;

	bool DrawEditor();

	void JsonSerialize(RE_Json* node) const override;
	void JsonDeserialize(RE_Json* node) override;

	size_t GetBinarySize() const override;
	void BinarySerialize(char*& cursor) const override;
	void BinaryDeserialize(char*& cursor) override;
};

#endif // !__RE_EMISSION_SHAPE_H__