#ifndef __RE_PARTICLEEMITTER_H__
#define __RE_PARTICLEEMITTER_H__

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Geometry/Plane.h"
#include <EASTL/vector.h>

#include "ImGui/imgui.h"

//#define Impulse_Thomas_Smid

class RE_CompPrimitive;

struct RE_ParticleEmitter
{
	unsigned int id = 0u;

	// Playback parameters
	enum PlaybackState { STOP = 0, PLAY, PAUSE } state = STOP;
	bool loop = true; // float start_delay = 0.0f;

	// Instantiation parameters
	float lifetime = 14.f;
	float speed_muliplier = 1.f;
	float spawn_frequency = 10.f;
	float spawn_offset = 0.f;

	inline int GetNewSpawns(float dt)
	{
		int units = static_cast<int>((spawn_offset += dt) * spawn_frequency);
		spawn_offset -= static_cast<float>(units) / spawn_frequency;
		return units;
	}

	// Control values
	float maxLifeTime = 15.5f;
	float maxSpeed = 20.f;
	float maxDistance = 2.f * 1.5f; //lifetime * speed

	// Physic properties ---------------------------------------------------------

	// Collider
	float restitution = 0.85f; // elastic vs inelastic

	// External Forces
	float gravity = -9.81f;
	math::vec wind = math::vec::zero;

	// Boundaries
	enum BoundaryType
	{
		NONE,
		GROUND,
		CEILING,
		BOX,
		SPHERE
	} bound_type = GROUND;

	enum BoundaryEffect
	{
		CONTAIN,
		KILL,
		CLAMP
	} effect;

	union Boundary
	{
		Boundary() {}
		float radius = 0.f; // height
		math::Plane plane;
	} boundary;

	float boundary_restitution = 0.95f;

	// Render properties ---------------------------------------------------------

	enum ColorState { SINGLE = 0, OVERLIFETIME, OVERDISTANCE, OVERSPEED };
	ColorState cState = SINGLE;
	math::vec particleColor = math::vec::one;
	math::vec gradient[2] = { math::vec::zero, math::vec::one };
	bool useOpacity = false, opacityWithCurve = false;  float opacity = 1.0f;

	bool emitlight = false;
	math::vec lightColor = math::vec::one;
	bool randomLightColor = false;
	float specular = 0.2f;
	float sClamp[2] = { 0.f, 1.f };
	bool randomSpecular = false;
	bool particleLColor = false;

	// Attenuattion
	float iClamp[2] = { 0.0f, 50.0f };
	float intensity = 1.0f;
	bool randomIntensity = false;
	float constant = 1.0f;
	float linear = 0.091f;
	float quadratic = 0.011f;

	const char* materialMD5 = nullptr;
	bool useTextures = false;
	const char* meshMD5 = nullptr;
	RE_CompPrimitive* primCmp = nullptr;

	math::float3 scale = { 0.5f,0.5f,0.1f };

	enum Particle_Dir : int
	{
		PS_FromPS,
		PS_Billboard,
		PS_Custom
	} particleDir = PS_Billboard;
	math::float3 direction = { -1.0f,1.0f,0.5f };

	//Curves
	bool useCurve = false;
	bool smoothCurve = false;
	eastl::vector< ImVec2> curve;
	int total_points = 10;
};

#endif //!__RE_PARTICLEEMITTER_H__