#ifndef __RE_PARTICLEEMITTER_H__
#define __RE_PARTICLEEMITTER_H__

#include "RE_EmissionData.h"

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
	unsigned int max_particles = 1000u;
	float maxLifeTime = 15.5f;
	float maxSpeed = 20.f;
	float maxDistance = 2.f * 1.5f; //lifetime * speed

	float spawn_frequency = 10.f;
	float spawn_offset = 0.f;

	// Instantiation parameters
	RE_EmissionLifetime lifetime = {};
	RE_EmissionShape init_pos = {};
	RE_EmissionSpeed init_speed = {};

	// Physic properties ---------------------------------------------------------
	
	RE_EmissionMass init_mass = {};
	RE_EmissionColRadius init_col_r = {};
	RE_EmissionColRest col_restitution = {};
	RE_EmissionBoundary boundary = {};
	RE_EmissionExternalForces external_acc = {};

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