#ifndef __RE_EMISSION_SHAPE_H__
#define __RE_EMISSION_SHAPE_H__

#include "MathGeoLib/include/Geometry/Circle.h"
#include "MathGeoLib/include/Geometry/Sphere.h"
#include "MathGeoLib/include/Geometry/Plane.h"
#include "MathGeoLib/include/Geometry/AABB.h"
#include "MathGeoLib/include/Math/float3.h"
#include "ImGui/imgui.h"
#include <EASTL/utility.h>
#include <EASTL/vector.h>

class RE_Json;

struct RE_EmissionInterval
{
	enum Type : int
	{
		NONE = 0,
		INTERMITENT,
		CUSTOM
	} type = NONE;

	bool is_open = false;
	float time_offset = 0.f;

	float duration[2] = { 1.f, 1.f };

	bool IsActive(float &dt);

	bool DrawEditor();

	void JsonDeserialize(RE_Json* node);
	void JsonSerialize(RE_Json* node) const;

	void BinaryDeserialize(char*& cursor);
	void BinarySerialize(char*& cursor) const;
	unsigned int GetBinarySize() const;
};

struct RE_EmissionSpawn
{
	enum Type : int
	{
		SINGLE = 0,
		BURST,
		FLOW
	} type = FLOW;

	bool has_started = false;
	float time_offset = 0.f;

	int particles_spawned = 10;
	float frequency = 10.f;

	bool DrawEditor();

	void JsonDeserialize(RE_Json* node);
	void JsonSerialize(RE_Json* node) const;

	void BinaryDeserialize(char*& cursor);
	void BinarySerialize(char*& cursor) const;
	unsigned int GetBinarySize() const;
};

struct RE_EmissionShape
{
	enum Type : int
	{
		POINT = 0,
		CIRCLE,
		RING,
		AABB,
		SPHERE,
		HOLLOW_SPHERE
	} type = CIRCLE;

	union Geo
	{
		math::vec point;
		math::Circle circle = math::Circle(math::vec::zero, { 0.f, 1.f, 0.f }, 1.f);
		eastl::pair<math::Circle, float> ring;
		math::AABB box;
		math::Sphere sphere;
		eastl::pair<math::Sphere, float> hollow_sphere;
	} geo = {};

	math::vec GetPosition() const;

	void DrawEditor();

	void JsonDeserialize(RE_Json* node);
	void JsonSerialize(RE_Json* node) const;

	void BinaryDeserialize(char*& cursor);
	void BinarySerialize(char*& cursor) const;
	unsigned int GetBinarySize() const;
};

struct RE_EmissionVector
{
	enum Type : int
	{
		NONE = 0,
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

	void JsonDeserialize(RE_Json* node);
	void JsonSerialize(RE_Json* node) const;

	void BinaryDeserialize(char*& cursor);
	void BinarySerialize(char*& cursor) const;
	unsigned int GetBinarySize() const;
};

struct RE_EmissionSingleValue
{
	enum Type : int
	{
		NONE = 0,
		VALUE,
		RANGE
	} type = VALUE;

	float val = 1.f;
	float margin = 1.f;

	float GetValue() const;
	float GetMin() const;
	float GetMax() const;

	void DrawEditor(const char* name);

	void JsonDeserialize(RE_Json* node);
	void JsonSerialize(RE_Json* node) const;

	void BinaryDeserialize(char*& cursor);
	void BinarySerialize(char*& cursor) const;
	unsigned int GetBinarySize() const;
};

struct RE_EmissionExternalForces
{
	enum Type : int
	{
		NONE = 0,
		GRAVITY,
		WIND,
		WIND_GRAVITY
	} type = GRAVITY;

	float gravity = -9.81f;
	math::vec wind = math::vec::zero;

	math::vec GetAcceleration() const;

	void DrawEditor();

	void JsonDeserialize(RE_Json* node);
	void JsonSerialize(RE_Json* node) const;

	void BinaryDeserialize(char*& cursor);
	void BinarySerialize(char*& cursor) const;
	unsigned int GetBinarySize() const;
};

struct RE_Particle;

struct RE_EmissionBoundary
{
	enum Type : int
	{
		NONE = 0,
		PLANE,
		SPHERE,
		AABB
	} type = NONE;

	enum Effect : int
	{
		CONTAIN = 0,
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

	void JsonDeserialize(RE_Json* node);
	void JsonSerialize(RE_Json* node) const;

	void BinaryDeserialize(char*& cursor);
	void BinarySerialize(char*& cursor) const;
	unsigned int GetBinarySize() const;
};

struct RE_EmissionCollider
{
	enum Type : int
	{
		NONE = 0,
		POINT,
		SPHERE
	} type = Type::NONE;

	bool inter_collisions = false;

	RE_EmissionSingleValue mass = {};
	RE_EmissionSingleValue radius = {};
	RE_EmissionSingleValue restitution = {};

	void DrawEditor();

	void JsonDeserialize(RE_Json* node);
	void JsonSerialize(RE_Json* node) const;

	void BinaryDeserialize(char*& cursor);
	void BinarySerialize(char*& cursor) const;
	unsigned int GetBinarySize() const;
};

struct CurveData
{
	CurveData();
	~CurveData();
	bool smooth = false;
	int total_points = 10;
	eastl::vector<ImVec2> points = {};
	int comboCurve = 0;

	float GetValue(const float weight) const;
	void DrawEditor(const char* name);
};

struct RE_PR_Color
{
	enum Type : int
	{
		SINGLE = 0,
		OVERLIFETIME,
		OVERDISTANCE,
		OVERSPEED
	} type = SINGLE;

	math::vec base = math::vec::one;
	math::vec gradient = math::vec::zero;

	bool useCurve = false;
	CurveData curve = {};

	math::vec GetValue(const float weight = 1.f) const;

	void DrawEditor();

	void JsonDeserialize(RE_Json* node);
	void JsonSerialize(RE_Json* node) const;

	void BinaryDeserialize(char*& cursor);
	void BinarySerialize(char*& cursor) const;
	unsigned int GetBinarySize() const;
};

struct RE_PR_Opacity
{
	enum Type : int
	{
		NONE = 0,
		VALUE,
		OVERLIFETIME,
		OVERDISTANCE,
		OVERSPEED
	} type = NONE;

	float opacity = 1.0f;
	bool inverted = false;

	bool useCurve = false;
	CurveData curve = {};

	float GetValue(const float weight) const;

	void DrawEditor();

	void JsonDeserialize(RE_Json* node);
	void JsonSerialize(RE_Json* node) const;

	void BinaryDeserialize(char*& cursor);
	void BinarySerialize(char*& cursor) const;
	unsigned int GetBinarySize() const;
};

#endif //!__RE_EMISSION_SHAPE_H__