#include "ParticleManager.h"

#include "RE_Math.h"
#include "RE_Particle.h"
#include <EASTL/vector.h>

unsigned int ParticleManager::emitter_count = 0u;

ParticleManager::ParticleManager()
{
	circle_precompute.reserve(static_cast<unsigned int>(circle_steps));
	const float interval = RE_Math::pi_x2 / circle_steps;
	for (float i = 0.f; i < RE_Math::pi_x2; i += interval)
		circle_precompute.push_back({ math::Sin(i), -math::Cos(i) });
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

int ParticleManager::GetCircleSteps() const
{
	return static_cast<int>(circle_steps);
}

void ParticleManager::SetCircleSteps(int steps)
{
	circle_precompute.clear();
	circle_precompute.set_capacity(static_cast<unsigned int>(steps));

	circle_steps = static_cast<float>(steps);
	const float interval = RE_Math::pi_x2 / circle_steps;
	for (float i = 0.f; i < RE_Math::pi_x2; i += interval)
		circle_precompute.push_back({ math::Sin(i), -math::Cos(i) });
}

void ParticleManager::DrawDebug() const
{
	const float interval = RE_Math::pi_x2 / circle_steps;
	eastl::list<eastl::pair<RE_ParticleEmitter*, eastl::list<RE_Particle*>*>*>::const_iterator it;
	for (it = simulations.cbegin(); it != simulations.cend(); ++it)
	{
		if ((*it)->first->active_physics)
		{
			const math::vec go_pos = math::vec::zero;

			// Render Particles
			for (auto p = (*it)->second->cbegin(); p != (*it)->second->cend(); ++p)
				DrawAASphere(go_pos + (*p)->position, (*p)->col_radius);

			// Render Boundary
			switch ((*it)->first->boundary.type)
			{
			case RE_EmissionBoundary::NONE: break;
			case RE_EmissionBoundary::PLANE:
			{
				const float interval = RE_Math::pi_x2 / circle_steps;
				for (float j = 1.f; j * j < (*it)->first->dist_range_sq[1]; ++j)
				{
					const math::Circle c = (*it)->first->boundary.geo.plane.GenerateCircle(go_pos, j * j);
					math::vec previous = c.GetPoint(0.f);
					for (float i = interval; i < RE_Math::pi_x2; i += interval)
					{
						glVertex3f(previous.x, previous.y, previous.z);
						previous = c.GetPoint(i);
						glVertex3f(previous.x, previous.y, previous.z);
					}
				}

				break; 
			}
			case RE_EmissionBoundary::SPHERE:
			{
				DrawAASphere(go_pos + (*it)->first->boundary.geo.sphere.pos, (*it)->first->boundary.geo.sphere.r);
				break; 
			}
			case RE_EmissionBoundary::AABB:
			{
				for (int i = 0; i < 12; i++)
				{
					glVertex3fv((*it)->first->boundary.geo.box.Edge(i).a.ptr());
					glVertex3fv((*it)->first->boundary.geo.box.Edge(i).b.ptr());
				}

				break;
			}
			}
		}
	}
}

void ParticleManager::DrawAASphere(const math::vec p_pos, const float radius) const
{
	eastl::vector<math::float2>::const_iterator i = circle_precompute.cbegin() + 1;
	math::vec previous = { p_pos.x + (circle_precompute.cbegin()->x * radius), p_pos.y + (circle_precompute.cbegin()->y * radius), p_pos.z };
	for (; i != circle_precompute.cend(); ++i) // Z
	{
		glVertex3fv(previous.ptr());
		previous = { p_pos.x + (i->x * radius), p_pos.y + (i->y * radius), p_pos.z };
		glVertex3fv(previous.ptr());
	}
	previous = { p_pos.x + (circle_precompute.cbegin()->x * radius), p_pos.y, p_pos.z + (circle_precompute.cbegin()->y * radius) };
	for (i = circle_precompute.cbegin() + 1; i != circle_precompute.cend(); ++i) // Y
	{
		glVertex3fv(previous.ptr());
		previous = { p_pos.x + (i->x * radius), p_pos.y, p_pos.z + (i->y * radius) };
		glVertex3fv(previous.ptr());
	}
	previous = { p_pos.x, p_pos.y + (circle_precompute.cbegin()->y * radius), p_pos.z + (circle_precompute.cbegin()->x * radius) };
	for (i = circle_precompute.cbegin() + 1; i != circle_precompute.cend(); ++i) // X
	{
		glVertex3fv(previous.ptr());
		previous = { p_pos.x, p_pos.y + (i->y * radius), p_pos.z + (i->x * radius) };
		glVertex3fv(previous.ptr());
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
