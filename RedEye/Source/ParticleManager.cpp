#include "ParticleManager.h"

#include "RE_Math.h"

unsigned int ParticleManager::emitter_count = 0u;

ParticleManager::ParticleManager()
{
}

ParticleManager::~ParticleManager()
{
	simulations.clear();
}

unsigned int ParticleManager::Allocate(RE_ParticleEmitter* emitter)
{
	simulations.push_back(new eastl::pair<RE_ParticleEmitter*, eastl::list<RE_Particle*>*>(emitter, new eastl::list<RE_Particle*>()));
	return (emitter->id = ++emitter_count);
}

bool ParticleManager::Deallocate(unsigned int index)
{
	eastl::list<eastl::pair<RE_ParticleEmitter*, eastl::list<RE_Particle*>*>*>::iterator it;
	for (it = simulations.begin(); it != simulations.end(); ++it)
	{
		if ((*it)->first->id == index)
		{
			simulations.erase(it);
			return true;
		}
	}

	return false;
}

bool ParticleManager::SetEmitterState(unsigned int index, RE_ParticleEmitter::PlaybackState state)
{
	eastl::list<eastl::pair<RE_ParticleEmitter*, eastl::list<RE_Particle*>*>*>::iterator it;
	for (it = simulations.begin(); it != simulations.end(); ++it)
	{
		if ((*it)->first->id == index)
		{
			(*it)->first->state = state;
			return true;
		}
	}

	return false;
}
