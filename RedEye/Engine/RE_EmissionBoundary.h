#ifndef __RE_EMISSION_BOUNDARY_H__
#define __RE_EMISSION_BOUNDARY_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <MGL/Geometry/Sphere.h>
#include <MGL/Geometry/Plane.h>
#include <MGL/Geometry/AABB.h>

struct RE_Particle;

struct RE_EmissionBoundary : RE_Serializable
{
	RE_EmissionBoundary() = default;

	enum class Type : ushort
	{
		NONE = 0,
		PLANE,
		SPHERE,
		AABB
	};

	Type type = Type(0);

	enum class Effect : ushort
	{
		CONTAIN = 0,
		KILL
	};

	Effect effect = Effect(0);

	union Data
	{
		math::Plane plane;
		math::Sphere sphere;
		math::AABB box;
	};

	Data geo = {};

	float restitution = 0.95f;

	bool HasBoundary() const;

	bool PointCollision(RE_Particle& p) const;
	bool SphereCollision(RE_Particle& p) const;

	bool DrawEditor();

	void JsonSerialize(RE_Json* node) const final;
	void JsonDeserialize(RE_Json* node) final;

	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor) const final;
	void BinaryDeserialize(char*& cursor) final;
};

#endif // !__RE_EMISSION_BOUNDARY_H__