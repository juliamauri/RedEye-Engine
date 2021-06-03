#include "RE_ParticleEmitter.h"

#include "Application.h"
#include "RE_Math.h"

void RE_ParticleEmitter::Update(const float global_dt)
{
	switch (state)
	{
	case RE_ParticleEmitter::STOPING:
	{
		Reset();
		state = RE_ParticleEmitter::STOP;
		break;
	}
	case RE_ParticleEmitter::RESTART:
	{
		Reset();
		state = RE_ParticleEmitter::PLAY;
		break;
	}
	case RE_ParticleEmitter::PLAY:
	{
		// Check time limitations
		float local_dt = global_dt * time_muliplier;
		if (total_time < start_delay)
		{
			if (total_time + local_dt >= start_delay)
			{
				total_time += local_dt;
				local_dt -= start_delay - total_time;
			}
			else break;
		}
		else total_time += local_dt;

		if (!loop && total_time >= max_time)
		{
			state = RE_ParticleEmitter::STOPING;
			break;
		}

		// Reset control values
		max_dist_sq = max_speed_sq = 0.f;

		// Update particles
		for (eastl::list<RE_Particle*>::iterator p1 = particle_pool.begin(); p1 != particle_pool.end();)
		{
			// Check if particle is still alive
			bool is_alive = true;
			switch (initial_lifetime.type) {
			case RE_EmissionSingleValue::Type::VALUE: is_alive = ((*p1)->lifetime += local_dt) < initial_lifetime.GetValue(); break;
			case RE_EmissionSingleValue::Type::RANGE: is_alive = ((*p1)->lifetime += local_dt) < (*p1)->max_lifetime; break;
			default: break; }

			// Iterate collisions
			if (is_alive)
			{
				switch (collider.shape) {
				case RE_EmissionCollider::Type::POINT:
				{
					if (collider.inter_collisions)
						for (eastl::list<RE_Particle*>::iterator p2 = p1.next(); p2 != particle_pool.end(); ++p2)
							ImpulseCollision(**p1, **p2);

					is_alive = boundary.PointCollision(**p1);
					break; 
				}
				case RE_EmissionCollider::Type::SPHERE:
				{
					if (collider.inter_collisions)
						for (eastl::list<RE_Particle*>::iterator p2 = p1.next(); p2 != particle_pool.end(); ++p2)
							ImpulseCollision(**p1, **p2, (*p1)->col_radius +(*p2)->col_radius);

					is_alive = boundary.SphereCollision(**p1); 
					break;
				}
				default: 
				{
					is_alive = boundary.PointCollision(**p1);
					break;
				}
				}
			}

			if (is_alive)
			{
				// Acceleration
				(*p1)->velocity += external_acc.GetAcceleration() * local_dt;

				/*/ Cap speed
				if (p.velocity.LengthSq() > emitter->maxSpeed * emitter->maxSpeed)
					p.velocity = p.velocity.Normalized() * emitter->maxSpeed;*/

					// Update position
				(*p1)->position += (*p1)->velocity * local_dt;

				// Update Control values
				max_dist_sq = RE_Math::MaxF(max_dist_sq, (*p1)->position.LengthSq());
				max_speed_sq = RE_Math::MaxF(max_speed_sq, (*p1)->velocity.LengthSq());

				++p1;
			}
			else // Remove dead particles
			{
				DEL(*p1);
				p1 = particle_pool.erase(p1);
				particle_count--;
			}
		}

		// Spawn new particles
		if (spawn_interval.IsActive(local_dt))
		{
			unsigned int to_add = RE_Math::MinUI(CountNewParticles(local_dt), max_particles - particle_count);
			particle_count += to_add;
			for (unsigned int i = 0u; i < to_add; ++i)
				particle_pool.push_back(SpawnParticle());
		}

		break;
	}
	default: break;
	}
}

void RE_ParticleEmitter::Reset()
{
	if (!particle_pool.empty()) particle_pool.clear();

	particle_count = 0u;
	max_dist_sq = max_speed_sq = total_time = 
	spawn_interval.time_offset = spawn_mode.time_offset = 0.f;
	spawn_mode.has_started = false;
}

unsigned int RE_ParticleEmitter::CountNewParticles(const float dt)
{
	unsigned int ret = 0u;

	spawn_mode.time_offset += dt;

	switch (spawn_mode.type)
	{
	case RE_EmissionSpawn::Type::SINGLE:
	{
		if (!spawn_mode.has_started)
			ret = static_cast<unsigned int>(spawn_mode.particles_spawned);

		break;
	}
	case RE_EmissionSpawn::Type::BURST:
	{
		int mult = static_cast<int>(!spawn_mode.has_started);
		while (spawn_mode.time_offset >= spawn_mode.frequency)
		{
			spawn_mode.time_offset -= spawn_mode.frequency;
			mult++;
		}

		ret += static_cast<unsigned int>(spawn_mode.particles_spawned * mult);

		break;
	}
	case RE_EmissionSpawn::Type::FLOW:
	{
		ret = static_cast<unsigned int>(spawn_mode.time_offset * spawn_mode.frequency);
		spawn_mode.time_offset -= static_cast<float>(ret) / spawn_mode.frequency;
		break;
	}
	}

	spawn_mode.has_started = true;

	return ret;
}

RE_Particle* RE_ParticleEmitter::SpawnParticle()
{
	RE_Particle* ret = new RE_Particle();

	// Set base properties
	ret->lifetime = 0.f;
	ret->max_lifetime = initial_lifetime.GetValue();
	//particle->dt_offset = dt - emitter->spawn_mode.time_offset - ((1.f / emitter->spawn_mode.frequency) * (i + 1));

	ret->position = initial_pos.GetPosition();
	ret->velocity = initial_speed.GetSpeed();

	// Set physic properties
	ret->mass = collider.mass.GetValue();
	ret->col_radius = collider.radius.GetValue();
	ret->col_restitution = collider.restitution.GetValue();

	// Set light properties
	ret->intensity = (randomIntensity) ? RE_MATH->RandomF(iClamp[0], iClamp[1]) : intensity;
	ret->specular = (randomSpecular) ? RE_MATH->RandomF(sClamp[0], sClamp[1]) : specular;

	if (randomLightColor)
		ret->lightColor.Set(RE_MATH->RandomF(), RE_MATH->RandomF(), RE_MATH->RandomF());
	else
		ret->lightColor = lightColor;

	return ret;
}

void RE_ParticleEmitter::ImpulseCollision(RE_Particle& p1, RE_Particle& p2, const float combined_radius) const
{
	// Check particle collision
	const math::vec collision_dir = p1.position - p2.position;
	const float dist2 = collision_dir.Dot(collision_dir);
	if (dist2 <= combined_radius * combined_radius)
	{
		// Get mtd: Minimum Translation Distance
		const float dist = math::Sqrt(dist2);
		const math::vec mtd = collision_dir * (combined_radius - dist) / dist;

		// Resolve Intersection
		const float p1_inv_mass = 1.f / p1.mass;
		const float p2_inv_mass = 1.f / p2.mass;
		p1.position += mtd * (p1_inv_mass / (p1_inv_mass + p2_inv_mass));
		p2.position -= mtd * (p2_inv_mass / (p1_inv_mass + p2_inv_mass));

		// Resolve Collision
		const math::vec col_normal = collision_dir.Normalized();
		const math::vec col_speed = (p1.velocity - p2.velocity);
		const float dot = col_speed.Dot(col_normal);

		// Check if particles not already moving away
		if (dot < 0.0f)
		{
			// Resolve applying impulse
			const math::vec impulse = col_normal * (-(p1.col_restitution + p2.col_restitution) * dot) / (p1_inv_mass + p2_inv_mass);
			p1.velocity += impulse * p1_inv_mass;
			p2.velocity -= impulse * p2_inv_mass;
		}
	}
}

