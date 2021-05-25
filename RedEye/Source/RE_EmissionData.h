#ifndef __RE_EMISSION_SHAPE_H__
#define __RE_EMISSION_SHAPE_H__

#include "MathGeoLib/include/Geometry/Circle.h"
#include "MathGeoLib/include/Geometry/Sphere.h"
#include "MathGeoLib/include/Geometry/Plane.h"
#include <EASTL/utility.h>

struct RE_EmissionShape
{
	enum Type : int
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
	} type = RANGEXYZ;

	math::vec val = -math::vec::one;
	math::vec dt = math::vec::one;

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
	float dt = 1.f;

	float GetValue() const;
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
	} type = GRAVITY;

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
		BOX
	} type = NONE;

	enum Effect : int
	{
		CONTAIN,
		KILL,
		CLAMP
	} effect = CONTAIN;

	union Data
	{
		math::Plane plane;
		math::Sphere sphere;
		math::vec box[2];
	} data;

	float restitution = 0.95f;

	bool ParticleCollision(RE_Particle& p) const;
	void DrawEditor();
};

#endif //!__RE_EMISSION_SHAPE_H__