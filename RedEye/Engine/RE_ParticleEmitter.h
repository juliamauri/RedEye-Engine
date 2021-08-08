#ifndef __RE_PARTICLEEMITTER_H__
#define __RE_PARTICLEEMITTER_H__

#include "RE_Profiler.h"
#include "RE_Particle.h"
#include "RE_EmissionData.h"

#include <EASTL/vector.h>

#if defined(PARTICLE_PHYSICS_TEST) || defined(PARTICLE_RENDER_TEST)
#include <EASTL/string.h>
#endif // PARTICLE_PHYSICS_TEST || PARTICLE_RENDER_TEST

class RE_CompPrimitive;

class RE_ParticleEmitter
{
public:
	RE_ParticleEmitter(bool instance_primitive = false);
	~RE_ParticleEmitter();

	unsigned int Update(const float global_dt);
	void Reset();

private:

	inline bool IsTimeValid(const float global_dt);
	inline void UpdateParticles();
	inline void UpdateSpawn();

	void ImpulseCollision(RE_Particle& p1, RE_Particle& p2, const float combined_radius = 0.001f) const;

public:

	unsigned int id = 0u;
	enum PlaybackState { STOPING, RESTART, PLAY, STOP, PAUSE } state = STOP;

	// Particle storage
	eastl::vector<RE_Particle> particle_pool;

	// Control (read-only)
	unsigned int particle_count = 0u;
	float total_time = 0.f;
	float max_dist_sq = 0.f;
	float max_speed_sq = 0.f;
	float local_dt = 0.f;
	math::vec parent_pos = math::vec::zero;
	math::vec parent_speed = math::vec::zero;

	math::AABB bounding_box;
	static enum BoundingMode : int { GENERAL, PER_PARTICLE } mode;

	// Emission properties ---------------------------------------------------------
	
	// Playback
	bool loop = true;
	float max_time = 5.f;
	float start_delay = 0.0f;
	float time_muliplier = 1.f;
	bool start_on_play = true;

	// Spawning
	unsigned int max_particles = 500000u;
	RE_EmissionInterval spawn_interval = {};
	RE_EmissionSpawn spawn_mode = {};

	// Instantiation
	RE_EmissionSingleValue initial_lifetime = {};
	RE_EmissionShape initial_pos = {};
	RE_EmissionVector initial_speed = {};

	// GO & Space
	bool local_space = true;
	bool inherit_speed = false;

	// Physics
	RE_EmissionExternalForces external_acc = {};
	RE_EmissionBoundary boundary = {};
	RE_EmissionCollider collider = {};

	// Render properties ---------------------------------------------------------

	bool active_rendering = true;

	math::float3 scale = { 0.5f,0.5f,0.1f };

	RE_PR_Color color = {};
	RE_PR_Opacity opacity = {};
	RE_PR_Light light = {};

	const char* meshMD5 = nullptr;
	RE_CompPrimitive* primCmp = nullptr;

	enum Particle_Dir : int
	{
		PS_FromPS,
		PS_Billboard,
		PS_Custom
	} particleDir = PS_Billboard;

	math::float3 direction = { -1.0f,1.0f,0.5f };

#if defined(PARTICLE_PHYSICS_TEST) || defined(PARTICLE_RENDER_TEST)

	void DemoSetup();

	static RE_ParticleEmitter* demo_emitter;
	static eastl::string filename;

#endif // PARTICLE_PHYSICS_TEST || PARTICLE_RENDER_TEST
};

#endif //!__RE_PARTICLEEMITTER_H__