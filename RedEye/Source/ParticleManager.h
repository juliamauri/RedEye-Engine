#ifndef __RE_PARTICLEMANAGER_H__
#define __RE_PARTICLEMANAGER_H__

#include "EASTL/list.h"
#include "MathGeoLib/include/Math/float3.h"

struct Particle
{
	math::vec position = math::vec::zero;
	float lifetime = -1.0f;
};

struct ParticleEmitter
{
	unsigned int id;

	// Simulation parameters
	enum PlaybackState { STOP = 0, PLAY, PAUSE } state = STOP;
	bool loop = true;
	float spaw_frequency = 20.0f;
	float lifetime = 1.5f;

	// Particles managed
	unsigned int index_min;
	unsigned int index_max;
};

class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	unsigned int Allocate(ParticleEmitter emitter);
	bool Deallocate(unsigned int index);

	bool SetEmitterState(unsigned int index, ParticleEmitter::PlaybackState state);

public:

	eastl::list<eastl::pair<ParticleEmitter, eastl::list<Particle>>> particles;

private:

	static unsigned int emitter_count;
};

#endif //!__RE_PARTICLEMANAGER_H__