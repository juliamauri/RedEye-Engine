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

	void DrawEditor();
	void DrawDebug() const;

	bool SetEmitterState(unsigned int index, RE_ParticleEmitter::PlaybackState state);

public:

	eastl::list<RE_ParticleEmitter*> simulations;

private:

	void DrawAASphere(const math::vec p_pos, const float radius) const;

private:

	static unsigned int emitter_count;

	float point_size = 2.f;
	float circle_steps = 12.f;
	eastl::vector<math::float2> circle_precompute;
};

#endif //!__RE_PARTICLEMANAGER_H__