#ifndef __RE_PARTICLEEMITTER_H__
#define __RE_PARTICLEEMITTER_H__

#include "RE_Particle.h"
#include "RE_EmissionData.h"

#include "MathGeoLib/include/Math/float3.h"
#include "ImGui/imgui.h"
#include "EA/EASTL/list.h"
#include <EASTL/vector.h>

class RE_CompPrimitive;

struct RE_ParticleEmitter
{
	unsigned int id = 0u;

	// Particle storage
	unsigned int max_particles = 1000u;
	eastl::list<RE_Particle*> particle_pool;

	// Playback
	enum PlaybackState { STOPING, RESTART, PLAY, STOP, PAUSE } state = STOP;
	bool loop = true;
	float max_time = 5.f;
	float start_delay = 0.0f;
	float time_muliplier = 1.f;

	// Control (read-only)
	unsigned int particle_count = 0u;
	float total_time = 0.f;
	float max_dist_sq = 0.f;
	float max_speed_sq = 0.f;

	// Spawning
	RE_EmissionInterval spawn_interval = {};
	RE_EmissionSpawn spawn_mode = {};

	// Instantiation
	RE_EmissionSingleValue initial_lifetime = {};
	RE_EmissionShape initial_pos = {};
	RE_EmissionVector initial_speed = {};

	// External Forces
	RE_EmissionExternalForces external_acc = {};

	// Boundary
	RE_EmissionBoundary boundary = {};

	// Collider
	RE_EmissionCollider collider = {};

	// Render properties ---------------------------------------------------------

	bool active_rendering = true;

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

	void Update(const float global_dt);

	void Reset();

	unsigned int CountNewParticles(const float dt);
	RE_Particle* SpawnParticle();

	void ImpulseCollision(RE_Particle& p1, RE_Particle& p2) const;
	void ImpulseCollisionTS(RE_Particle& p1, RE_Particle& p2, const float dt) const;

};

#endif //!__RE_PARTICLEEMITTER_H__