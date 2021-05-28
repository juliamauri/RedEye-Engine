#ifndef __RE_PARTICLEEMITTER_H__
#define __RE_PARTICLEEMITTER_H__

#include "RE_EmissionData.h"

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"
#include "ImGui/imgui.h"
#include <EASTL/vector.h>

class RE_CompPrimitive;

struct RE_ParticleEmitter
{
	unsigned int id = 0u;

	// Playback parameters
	enum PlaybackState { STOP = 0, PLAY, PAUSE } state = STOP;
	bool loop = true; // float start_delay = 0.0f;
	float speed_muliplier = 1.f;

	// Control values
	unsigned int particle_count = 0u;
	unsigned int max_particles = 1000u;
	math::float2 dist_range_sq = math::float2::zero;
	math::float2 speed_range_sq = math::float2::zero;

	// Instantiation parameters
	float spawn_frequency = 10.f;
	float spawn_offset = 0.f;

	RE_EmissionLifetime initial_lifetime = {};
	RE_EmissionShape initial_pos = {};
	RE_EmissionSpeed initial_speed = {};

	// Acceleration
	RE_EmissionExternalForces external_acc = {};

	// Physic properties ---------------------------------------------------------
	
	bool active_physics = false;

	RE_EmissionMass initial_mass = {};
	RE_EmissionColRadius initial_col_radius = {};
	RE_EmissionColRest initial_col_restitution = {};

	RE_EmissionBoundary boundary = {};

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