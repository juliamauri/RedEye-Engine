#ifndef __RE_PARTICLEEMITTER_H__
#define __RE_PARTICLEEMITTER_H__

#include "RE_Particle.h"
#include "RE_EmissionInterval.h"
#include "RE_EmissionSpawn.h"
#include "RE_EmissionSingleValue.h"
#include "RE_EmissionShape.h"
#include "RE_EmissionVector.h"
#include "RE_EmissionExternalForces.h"
#include "RE_EmissionBoundary.h"
#include "RE_EmissionCollider.h"
#include "RE_PR_Color.h"
#include "RE_PR_Opacity.h"
#include "RE_PR_Light.h"

#include <EASTL/vector.h>

class RE_CompPrimitive;

class RE_ParticleEmitter
{
public:

	enum class PlaybackState : int { STOPING, RESTART, PLAY, STOP, PAUSE };
	enum class ParticleDir : uchar { FromPS, Billboard, Custom };

	PlaybackState state = PlaybackState::STOP;

	// Particle storage
	eastl::vector<RE_Particle> particle_pool;

	// Control (read-only)
	uint particle_count = 0;
	float total_time = 0.f;
	float max_dist_sq = 0.f;
	float max_speed_sq = 0.f;
	float local_dt = 0.f;
	math::vec parent_pos = math::vec::zero;
	math::vec parent_speed = math::vec::zero;

	math::AABB bounding_box;

#pragma region Emission Properties

	// Playback
	bool loop = true;
	float max_time = 5.f;
	float start_delay = 0.0f;
	float time_muliplier = 1.f;
	bool start_on_play = true;

	// Spawning
	uint max_particles = 500000;
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

#pragma endregion

#pragma region Render Properties

	bool active_rendering = true;

	math::float3 scale = { 0.5f,0.5f,0.1f };

	RE_PR_Color color = {};
	RE_PR_Opacity opacity = {};
	RE_PR_Light light = {};

	const char* meshMD5 = nullptr;
	RE_CompPrimitive* primCmp = nullptr;

	ParticleDir orientation = ParticleDir::Billboard;

	math::float3 direction = { -1.0f,1.0f,0.5f };

#pragma endregion

public:

	RE_ParticleEmitter(bool instance_primitive = false);
	~RE_ParticleEmitter();

	uint Update(const float global_dt);
	void Reset();

	bool HasLight() const { return light.HasLight(); }

private:

	inline bool IsTimeValid(const float global_dt);
	inline void UpdateParticles();
	inline void UpdateSpawn();

	void ImpulseCollision(RE_Particle& p1, RE_Particle& p2, const float combined_radius = 0.001f) const;
};

#endif //!__RE_PARTICLEEMITTER_H__