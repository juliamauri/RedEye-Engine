#include "ParticleManager.h"

#include "Application.h"
#include "RE_Math.h"

unsigned int ParticleManager::emitter_count = 0u;

bool RE_Particle::Update(float dt)
{
	bool ret = false;
	lifetime += dt;

	if (lifetime < max_lifetime)
	{
		for (int i = 0; i < 3; ++i)
			position[i] += dt * RE_MATH->RandomF();

		ret = true;
	}

	return ret;
}

int RE_ParticleEmitter::GetNewSpawns(float dt)
{
	int units = static_cast<int>((spawn_offset += dt) * spaw_frequency);
	spawn_offset -= static_cast<float>(units) / spaw_frequency;
	return units;
}

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
