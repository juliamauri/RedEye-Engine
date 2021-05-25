#ifndef __RE_EMISSION_SHAPE_H__
#define __RE_EMISSION_SHAPE_H__

#include "MathGeoLib/include/Geometry/Circle.h"
#include "MathGeoLib/include/Geometry/Sphere.h"
#include "MathGeoLib/include/Geometry/Plane.h"
#include <EASTL/utility.h>

struct RE_EmissionShape
{
	enum Type
	{
		POINT,
		BOX,
		CIRCLE,
		SPHERE,
		HOLLOW_SPHERE
	} shape = POINT;

	union Geo
	{
		math::vec point = math::vec::zero;
		math::Circle circle;
		math::vec box[2];
		math::Sphere sphere;
		eastl::pair<math::Sphere, float> hollow_sphere;
	} geo;

	math::vec GetPosition() const;
};

struct RE_EmissionVector
{
	enum Type
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
	} type = RANGEXYZ;

	math::vec min = -math::vec::one;
	math::vec max = math::vec::one;

	math::vec GetSpeed() const;
};

typedef RE_EmissionVector RE_EmissionSpeed;

struct RE_EmissionSingleValue
{
	enum Type
	{
		NONE,
		VALUE,
		RANGE
	} type = VALUE;

	float min = 1.f;
	float max = 1.f;

	float GetValue() const;
};

typedef RE_EmissionSingleValue RE_EmissionLifetime;
typedef RE_EmissionSingleValue RE_EmissionMass;
typedef RE_EmissionSingleValue RE_EmissionColRadius;
typedef RE_EmissionSingleValue RE_EmissionColRest;

struct RE_EmissionExternalForces
{
	enum Type
	{
		NONE,
		GRAVITY,
		WIND,
		WIND_GRAVITY
	} type = GRAVITY;

	float gravity = -9.81f;
	math::vec wind = math::vec::zero;

	math::vec GetAcceleration() const;
};

struct RE_Particle;

struct RE_EmissionBoundary
{
	enum Type
	{
		NONE,
		GROUND,
		CEILING,
		BOX,
		SPHERE
	} type = NONE;

	enum Effect
	{
		CONTAIN,
		KILL,
		CLAMP
	} effect = CONTAIN;

	union Data
	{
		float radius = 0.f; // height
		math::Plane plane;
	} data;

	float restitution = 0.95f;

	void ParticleCollision(RE_Particle& p) const;
};

#endif //!__RE_EMISSION_SHAPE_H__