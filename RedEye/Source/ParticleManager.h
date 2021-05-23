#ifndef __RE_PARTICLEMANAGER_H__
#define __RE_PARTICLEMANAGER_H__

#include "RE_ParticleEmitter.h"
#include "EA/EASTL/utility.h"
#include "EA/EASTL/list.h"

struct RE_Particle;

class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	unsigned int Allocate(RE_ParticleEmitter* emitter);
	bool Deallocate(unsigned int index);

	void DrawDebug(float circle_steps) const;

	bool SetEmitterState(unsigned int index, RE_ParticleEmitter::PlaybackState state);

public:

	eastl::list<eastl::pair<RE_ParticleEmitter*, eastl::list<RE_Particle*>*>*> simulations;

private:

	static unsigned int emitter_count;
};

#endif //!__RE_PARTICLEMANAGER_H__