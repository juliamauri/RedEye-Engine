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
	simulations.push_back(emitter);
	return (emitter->id = (++emitter_count));
}

bool ParticleManager::Deallocate(unsigned int index)
{
	eastl::list<RE_ParticleEmitter*>::iterator it;
	for (it = simulations.begin(); it != simulations.end(); ++it)
	{
		if ((*it)->id == index)
		{
			simulations.erase(it);
			return true;
		}
	}

	return false;
}

void ParticleManager::DrawEditor()
{
	ImGui::DragFloat("Point size", &point_size, 1.f, 0.f, 100.f);

	int tmp = static_cast<int>(circle_steps);;
	if (ImGui::DragInt("Steps", &tmp, 1.f, 0, 64))
	{
		circle_precompute.clear();
		circle_precompute.set_capacity(static_cast<unsigned int>(tmp));

		circle_steps = static_cast<float>(tmp);
		const float interval = RE_Math::pi_x2 / circle_steps;
		for (float i = 0.f; i < RE_Math::pi_x2; i += interval)
			circle_precompute.push_back({ math::Sin(i), -math::Cos(i) });
	}
}

void ParticleManager::DrawDebug() const
{
	const float interval = RE_Math::pi_x2 / circle_steps;
	for (auto sim : simulations)
	{
		const math::vec go_pos = math::vec::zero;

		if (sim->initial_pos.shape || sim->boundary.type)
		{
			glBegin(GL_LINES);

			// Render Spawn Shape
			glColor4f(1.0f, 0.27f, 0.f, 1.f); // orange
			switch (sim->initial_pos.shape) {
			case RE_EmissionShape::Type::CIRCLE:
			{
				math::Circle c = sim->initial_pos.geo.circle;
				c.pos += go_pos;
				math::vec previous = c.GetPoint(0.f);
				for (float i = interval; i < RE_Math::pi_x2; i += interval)
				{
					glVertex3f(previous.x, previous.y, previous.z);
					previous = c.GetPoint(i);
					glVertex3f(previous.x, previous.y, previous.z);
				}
				break;
			}
			case RE_EmissionShape::Type::RING:
			{
				math::Circle c = sim->initial_pos.geo.ring.first;
				c.pos += go_pos;
				c.r += sim->initial_pos.geo.ring.second;
				math::vec previous = c.GetPoint(0.f);
				for (float i = interval; i < RE_Math::pi_x2; i += interval)
				{
					glVertex3f(previous.x, previous.y, previous.z);
					previous = c.GetPoint(i);
					glVertex3f(previous.x, previous.y, previous.z);
				}

				c.r -= 2.f * sim->initial_pos.geo.ring.second;
				previous = c.GetPoint(0.f);
				for (float i = interval; i < RE_Math::pi_x2; i += interval)
				{
					glVertex3f(previous.x, previous.y, previous.z);
					previous = c.GetPoint(i);
					glVertex3f(previous.x, previous.y, previous.z);
				}
				break;
			}
			case RE_EmissionShape::Type::AABB:
			{
				for (int i = 0; i < 12; i++)
				{
					glVertex3fv(sim->initial_pos.geo.box.Edge(i).a.ptr());
					glVertex3fv(sim->initial_pos.geo.box.Edge(i).b.ptr());
				}

				break;
			}
			case RE_EmissionShape::Type::SPHERE:
			{
				DrawAASphere(go_pos + sim->initial_pos.geo.sphere.pos, sim->initial_pos.geo.sphere.r);
				break;
			}
			case RE_EmissionShape::Type::HOLLOW_SPHERE:
			{
				DrawAASphere(go_pos + sim->initial_pos.geo.hollow_sphere.first.pos, sim->initial_pos.geo.hollow_sphere.first.r - sim->initial_pos.geo.hollow_sphere.second);
				DrawAASphere(go_pos + sim->initial_pos.geo.hollow_sphere.first.pos, sim->initial_pos.geo.hollow_sphere.first.r + sim->initial_pos.geo.hollow_sphere.second);
				break;
			}
			default: break;
			}

			// Render Boundary
			glColor4f(1.f, 0.84f, 0.0f, 1.f); // gold
			switch (sim->boundary.type) {
			case RE_EmissionBoundary::PLANE:
			{
				const float interval = RE_Math::pi_x2 / circle_steps;
				for (float j = 1.f; j < 6.f; ++j)
				{
					const math::Circle c = sim->boundary.geo.plane.GenerateCircle(go_pos, j * j);
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
				DrawAASphere(go_pos + sim->boundary.geo.sphere.pos, sim->boundary.geo.sphere.r);
				break;
			}
			case RE_EmissionBoundary::AABB:
			{
				for (int i = 0; i < 12; i++)
				{
					glVertex3fv(sim->boundary.geo.box.Edge(i).a.ptr());
					glVertex3fv(sim->boundary.geo.box.Edge(i).b.ptr());
				}

				break;
			}
			default: break;
			}

			// Render Shape Collider
			if (sim->collider.shape == RE_EmissionCollider::Type::SPHERE)
			{
				glColor4f(0.1f, 0.8f, 0.1f, 1.f); // light green
				for (auto p : sim->particle_pool)
					DrawAASphere(go_pos + p->position, p->col_radius);
			}

			glEnd();
		}

		// Render Point Collider
		if (sim->collider.shape == RE_EmissionCollider::Type::POINT)
		{
			glPointSize(point_size);
			glBegin(GL_POINTS);
			glColor4f(0.1f, 0.8f, 0.1f, 1.f); // light green

			for (auto p : sim->particle_pool)
			{
				const math::vec pos = go_pos + p->position;
				glVertex3fv(pos.ptr());
			}

			glPointSize(1.f);
			glEnd();
		}
	}
}

bool ParticleManager::SetEmitterState(unsigned int index, RE_ParticleEmitter::PlaybackState state)
{
	eastl::list<eastl::pair<RE_ParticleEmitter*, eastl::list<RE_Particle*>*>*>::iterator it;
	for (auto sim : simulations)
	{
		if (sim->id == index)
		{
			sim->state = state;
			return true;
		}
	}

	return false;
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
