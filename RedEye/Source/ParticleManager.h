#ifndef __RE_PARTICLEMANAGER_H__
#define __RE_PARTICLEMANAGER_H__

#include "MathGeoLib/include/Math/float3.h"
#include "EA/EASTL/utility.h"
#include "EA/EASTL/list.h"

struct RE_Particle
{
	bool Update(float dt);

	math::vec position = math::vec::zero;
	math::vec speed = math::vec::zero;

	float lifetime = 0.0f;
	float max_lifetime = 15.0f;
};

struct RE_ParticleEmitter
{
	int GetNewSpawns(float dt);

	unsigned int id = 0u;

	// Simulation parameters
	enum PlaybackState { STOP = 0, PLAY, PAUSE } state = STOP;
	bool loop = true;
	float spaw_frequency = 3.f;
	float lifetime = 1.5f;

	// Control values
	float spawn_offset = 0.f;
	float speed_muliplier = 2.f;
};

class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	unsigned int Allocate(RE_ParticleEmitter* emitter);
	bool Deallocate(unsigned int index);


	bool SetEmitterState(unsigned int index, RE_ParticleEmitter::PlaybackState state);

public:

	eastl::list<eastl::pair<RE_ParticleEmitter*, eastl::list<RE_Particle*>*>*> simulations;

private:

	static unsigned int emitter_count;
};

#endif //!__RE_PARTICLEMANAGER_H__