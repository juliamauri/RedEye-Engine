#ifndef __RE_EMISSION_SHAPE_H__
#define __RE_EMISSION_SHAPE_H__

#include "MathGeoLib/include/Geometry/Circle.h"
#include "MathGeoLib/include/Geometry/Sphere.h"
#include "MathGeoLib/include/Geometry/Plane.h"
#include "MathGeoLib/include/Geometry/AABB.h"
#include <EASTL/utility.h>

struct RE_EmissionInterval
{
	enum Type : int
	{
		NONE,
		INTERMITENT,
		CUSTOM
	} type = NONE;

	bool is_open = true;
	float time_offset = 0.f;
	float duration[2] = { 1.f, 1.f };

	bool IsActive(float &dt);
	bool DrawEditor();
};

struct RE_EmissionSpawn
{
	enum Type : int
	{
		SINGLE,
		BURST,
		FLOW
	} type = SINGLE;

	bool has_started = false;
	int particles_spawned = 10;
	float frequency = 10.f;
	float time_offset = 0.f;

	bool DrawEditor();
};

struct RE_EmissionShape
{
	enum Type : int
	{
		POINT,
		CIRCLE,
		RING,
		AABB,
		SPHERE,
		HOLLOW_SPHERE
	} shape = POINT;

	union Geo
	{
		math::vec point = math::vec::zero;
		math::Circle circle;
		eastl::pair<math::Circle, float> ring;
		math::AABB box;
		math::Sphere sphere;
		eastl::pair<math::Sphere, float> hollow_sphere;
	} geo = {};

	math::vec GetPosition() const;
	void DrawEditor();
};

struct RE_EmissionVector
{
	enum Type : int
	{
		NONE,
		VALUE,
		RANGEX,
		RANGEY,
		RANGEZ,
		RANGEXY,
		RANGEXZ,
		RANGEYZ,
		RANGEXYZ
	} type = NONE;

	math::vec val = -math::vec::one;
	math::vec margin = math::vec::one;

	math::vec GetSpeed() const;
	void DrawEditor(const char* name);
};

typedef RE_EmissionVector RE_EmissionSpeed;

struct RE_EmissionSingleValue
{
	enum Type : int
	{
		NONE,
		VALUE,
		RANGE
	} type = VALUE;

	float val = 1.f;
	float margin = 1.f;

	float GetValue() const;
	float GetMin() const;
	float GetMax() const;
	void DrawEditor(const char* name);
};

typedef RE_EmissionSingleValue RE_EmissionLifetime;
typedef RE_EmissionSingleValue RE_EmissionMass;
typedef RE_EmissionSingleValue RE_EmissionColRadius;
typedef RE_EmissionSingleValue RE_EmissionColRest;

struct RE_EmissionExternalForces
{
	enum Type : int
	{
		NONE,
		GRAVITY,
		WIND,
		WIND_GRAVITY
	} type = NONE;

	float gravity = -9.81f;
	math::vec wind = math::vec::zero;

	math::vec GetAcceleration() const;
	void DrawEditor();
};

struct RE_Particle;

struct RE_EmissionBoundary
{
	enum Type : int
	{
		NONE,
		PLANE,
		SPHERE,
		AABB
	} type = NONE;

	enum Effect : int
	{
		CONTAIN,
		KILL
	} effect = CONTAIN;

	union Data
	{
		math::Plane plane;
		math::Sphere sphere;
		math::AABB box;
	} geo;

	float restitution = 0.95f;

	bool PointCollision(RE_Particle& p) const;
	bool SphereCollision(RE_Particle& p) const;

	void DrawEditor();
};

#endif //!__RE_EMISSION_SHAPE_H__