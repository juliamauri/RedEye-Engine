#include "ParticleManager.h"

#include "RE_Math.h"
#include "RE_Particle.h"
#include <EASTL/vector.h>

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

void ParticleManager::DrawDebug(float circle_steps) const
{
	const float cap = math::pi * 2.f;
	const float interval = cap / circle_steps;

	eastl::list<eastl::pair<RE_ParticleEmitter*, eastl::list<RE_Particle*>*>*>::const_iterator it;
	for (it = simulations.cbegin(); it != simulations.cend(); ++it)
	{
		math::vec go_pos = math::vec::zero;

		for (auto p = (*it)->second->cbegin(); p != (*it)->second->cend(); ++p)
		{
			eastl::vector<math::float2> trigonometry;
			trigonometry.reserve(static_cast<unsigned int>(circle_steps));
			const float radius = (*p)->col_radius;
			const math::vec p_pos = go_pos + (*p)->position;
			for (float i = 0.f; i < cap; i += interval) trigonometry.push_back({ radius * sin(i), -radius * cos(i) });

			// Z
			eastl::vector<math::float2>::const_iterator i = trigonometry.cbegin() + 1;
			math::vec previous = { p_pos.x + trigonometry.cbegin()->x, p_pos.y + trigonometry.cbegin()->y, p_pos.z };
			for (; i != trigonometry.cend(); ++i)
			{
				glVertex3f(previous.x, previous.y, previous.z);
				previous = { p_pos.x + i->x, p_pos.y + i->y, p_pos.z };
				glVertex3f(previous.x, previous.y, previous.z);
			}
			// Y
			previous = { p_pos.x + trigonometry.cbegin()->x, p_pos.y, p_pos.z + trigonometry.cbegin()->y };
			for (i = trigonometry.cbegin() + 1; i != trigonometry.cend(); ++i)
			{
				glVertex3f(previous.x, previous.y, previous.z);
				previous = { p_pos.x + i->x, p_pos.y, p_pos.z + i->y };
				glVertex3f(previous.x, previous.y, previous.z);
			}
			// X
			previous = { p_pos.x, p_pos.y + trigonometry.cbegin()->y, p_pos.z + trigonometry.cbegin()->x };
			for (i = trigonometry.cbegin() + 1; i != trigonometry.cend(); ++i)
			{
				glVertex3f(previous.x, previous.y, previous.z);
				previous = { p_pos.x, p_pos.y + i->y, p_pos.z + i->x };
				glVertex3f(previous.x, previous.y, previous.z);
			}
		}
	}
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
