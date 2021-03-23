#include "ParticleManager.h"

unsigned int ParticleManager::emitter_count = 0u;

ParticleManager::ParticleManager()
{
}

ParticleManager::~ParticleManager()
{
	particles.clear();
}

unsigned int ParticleManager::Allocate(ParticleEmitter emitter)
{
	unsigned int ret = emitter.id = emitter_count++;
	particles.push_back({ emitter, {} });
	return ret;
}

bool ParticleManager::Deallocate(unsigned int index)
{
	eastl::list<eastl::pair<ParticleEmitter, eastl::list<Particle>>>::iterator it;
	for (it = particles.begin(); it != particles.end(); ++it)
	{
		if (it->first.id == index)
		{
			particles.erase(it);
			return true;
		}
	}

	return false;
}

bool ParticleManager::SetEmitterState(unsigned int index, ParticleEmitter::PlaybackState state)
{
	eastl::list<eastl::pair<ParticleEmitter, eastl::list<Particle>>>::iterator it;
	for (it = particles.begin(); it != particles.end(); ++it)
	{
		if (it->first.id == index)
		{
			it->first.state = state;
			return true;
		}
	}

	return false;
}
