#ifndef __RE_EMISSION_BOUNDARY_H__
#define __RE_EMISSION_BOUNDARY_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <MGL/Geometry/Sphere.h>
#include <MGL/Geometry/Plane.h>
#include <MGL/Geometry/AABB.h>

struct RE_Particle;

class RE_EmissionBoundary : public RE_Serializable
{
public:

	enum class Type : ushort
	{
		NONE = 0,
		PLANE,
		SPHERE,
		AABB
	};

	enum class Effect : ushort
	{
		CONTAIN = 0,
		KILL
	};

	union Data
	{
		math::Plane plane;
		math::Sphere sphere;
		math::AABB box;
	};

public:

	Type type = Type(0);
	Effect effect = Effect(0);
	Data geo = {};

	float restitution = 0.95f;

public:

	RE_EmissionBoundary() = default;
	~RE_EmissionBoundary() = default;

	bool DrawEditor();

	bool HasBoundary() const { return type != Type::NONE; }
	bool PointCollision(RE_Particle& p) const;
	bool SphereCollision(RE_Particle& p) const;

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_EMISSION_BOUNDARY_H__