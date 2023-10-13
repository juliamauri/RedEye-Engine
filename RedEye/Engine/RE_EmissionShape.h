#ifndef __RE_EMISSION_SHAPE_H__
#define __RE_EMISSION_SHAPE_H__

#include "RE_Serializable.h"
#include "RE_DataTypes.h"

#include <MGL/Geometry/AABB.h>
#include <MGL/Geometry/Circle.h>
#include <MGL/Geometry/Sphere.h>
#include <EASTL/vector.h>

class RE_EmissionShape : public RE_Serializable
{
public:

	enum class Type : ushort
	{
		POINT = 0,
		CIRCLE,
		RING,
		AABB,
		SPHERE,
		HOLLOW_SPHERE
	};

	union Geo
	{
		math::vec point;
		math::Circle circle = math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f);
		eastl::pair<math::Circle, float> ring;
		math::AABB box;
		math::Sphere sphere;
		eastl::pair<math::Sphere, float> hollow_sphere;
	};

public:

	Type type = Type(0);
	Geo geo = {};

public:

	RE_EmissionShape() = default;
	~RE_EmissionShape() = default;

	bool DrawEditor();

	math::vec GetPosition() const;
	bool IsShaped() const { return type != Type::POINT; }

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources = nullptr) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources = nullptr) final;
	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources = nullptr) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources = nullptr) final;
};

#endif // !__RE_EMISSION_SHAPE_H__