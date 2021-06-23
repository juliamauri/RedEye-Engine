#ifndef __RE_PARTICLEMANAGER_H__
#define __RE_PARTICLEMANAGER_H__

#include "RE_ParticleEmitter.h"

struct RE_Particle;

class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	void Update(const float dt);
	void Clear();

	void DrawSimulation(unsigned int index, math::float3  go_positon, math::float3  go_up) const;
	void CallLightShaderUniforms(unsigned int index, math::float3 go_position, unsigned int shader, const char* array_unif_name, unsigned int& count, unsigned int maxLights, bool sharedLight) const;

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

	unsigned int emitter_count = 0u;
	unsigned int particle_count = 0u;

	float point_size = 2.f;
	float circle_steps = 12.f;
	eastl::vector<math::float2> circle_precompute;
};

#endif //!__RE_PARTICLEMANAGER_H__