#ifndef __RE_PARTICLEMANAGER_H__
#define __RE_PARTICLEMANAGER_H__

#include "RE_ParticleEmitter.h"

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

	eastl::list<RE_ParticleEmitter*> simulations;

private:

	static unsigned int emitter_count;

	float circle_steps = 12.f;
	eastl::vector<math::float2> circle_precompute;
};

#endif //!__RE_PARTICLEMANAGER_H__