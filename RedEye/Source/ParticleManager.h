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

	int GetCircleSteps() const;
	void SetCircleSteps(int steps);

	void DrawDebug() const;
	void DrawAASphere(const math::vec p_pos, const float radius) const;

	bool SetEmitterState(unsigned int index, RE_ParticleEmitter::PlaybackState state);

public:

	eastl::list<eastl::pair<RE_ParticleEmitter*, eastl::list<RE_Particle*>*>*> simulations;

private:

	static unsigned int emitter_count;

	float circle_steps = 12.f;
	eastl::vector<math::float2> circle_precompute;
};

#endif //!__RE_PARTICLEMANAGER_H__