#include "ModulePhysics.h"

#include "Application.h"
#include "RE_Time.h"

ModulePhysics::ModulePhysics() : Module("Physics") {}
ModulePhysics::~ModulePhysics() {}

bool ModulePhysics::Init()
{
	return true;
}

bool ModulePhysics::Start()
{
	return true;
}

void ModulePhysics::Update()
{
	for (auto sim : particles.simulations)
	{
		if (sim->first->state == RE_ParticleEmitter::PLAY)
		{
			// Spawn new particles
			float dt = RE_TIME->GetDeltaTime();
			int to_add = sim->first->GetNewSpawns(dt);

			for (int i = 0; i < to_add; ++i)
				sim->second->push_back(new RE_Particle());

			for (eastl::list<RE_Particle*>::iterator it = sim->second->begin(); it != sim->second->end();)
			{
				if (!(*it)->Update(dt))
				{
					DEL(*it);
					it = sim->second->erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}
}

void ModulePhysics::CleanUp()
{
	particles.simulations.clear();
}

RE_ParticleEmitter* ModulePhysics::AddEmitter()
{
	RE_ParticleEmitter* ret = new RE_ParticleEmitter();
	particles.Allocate(ret);
	return ret;
}

void ModulePhysics::RemoveEmitter(RE_ParticleEmitter* emitter)
{
	particles.Deallocate(emitter->id);
}

unsigned int ModulePhysics::GetParticleCount(unsigned int emitter_id) const
{
	for (auto sim : particles.simulations)
	{
		if (sim->first->id == emitter_id)
		{
			return sim->second->size();
		}
	}
	return 0u;
}
