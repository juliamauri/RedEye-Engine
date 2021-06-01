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
		bool is_starting = false;
		float local_dt = global_dt * time_muliplier;
		if (total_time < start_delay)
		{
			if (total_time + local_dt >= start_delay)
			{
				is_starting = true;
				total_time += local_dt;
				local_dt -= start_delay - total_time;
			}
			else break;
		}
		else total_time += local_dt;

		if (!loop && total_time >= max_time)
		{
			state = RE_ParticleEmitter::STOP;
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
			default: break;
			}

			if (is_alive)
			{
				// Iterate for collisions (other particles could be dead, but dt should always be too small to notice)
				if (active_collider)
				{
					for (eastl::list<RE_Particle*>::iterator p2 = p1.next(); p2 != particle_pool.end(); ++p2)
					{
						switch (method) {
						case CollisionResolution::SIMPLE: ImpulseCollision(**p1, **p2); break;
						case CollisionResolution::Thomas_Smid: ImpulseCollisionTS(**p1, **p2, local_dt); break; }
					}

					is_alive = boundary.SphereCollision(**p1);
				}
				else is_alive = boundary.PointCollision(**p1);
			}

			if (is_alive)
			{
				// Acceleration
				(*p1)->velocity += external_acc.GetAcceleration() * local_dt;
				(*p1)->dt_offset = 0.f;

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
	ret->mass = initial_mass.GetValue();
	ret->col_radius = initial_col_radius.GetValue();
	ret->col_restitution = initial_col_restitution.GetValue();

	// Set light properties
	ret->intensity = (randomIntensity) ? RE_MATH->RandomF(iClamp[0], iClamp[1]) : intensity;
	ret->specular = (randomSpecular) ? RE_MATH->RandomF(sClamp[0], sClamp[1]) : specular;

	if (randomLightColor)
		ret->lightColor.Set(RE_MATH->RandomF(), RE_MATH->RandomF(), RE_MATH->RandomF());
	else
		ret->lightColor = lightColor;

	return ret;
}

void RE_ParticleEmitter::ImpulseCollision(RE_Particle& p1, RE_Particle& p2) const
{
	// Check particle collision
	const math::vec collision_dir = p1.position - p2.position;
	const float dist2 = collision_dir.Dot(collision_dir);
	const float combined_radius = p1.col_radius + p2.col_radius;
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

void RE_ParticleEmitter::ImpulseCollisionTS(RE_Particle& p1, RE_Particle& p2, const float dt) const
{
	// Adaptation to Thomas Smid's sphere collision detection and resolution
	//		Shifts coordinate system to check impending collisions

	// Get relative distance & speed
	float rel_distance = (p2.position - p1.position).Length();
	float rel_speed = (p2.velocity - p1.velocity).Length();

	// Check particle collision
	if (rel_distance / 1.2f <= p1.col_radius + p2.col_radius
		&& rel_speed > 0.000001f)
	{
		// Set p1 position as origin
		math::vec p2_rel_pos = p2.position - p1.position;

		// Get polar coordinates
		const float theta2 = math::Acos(p2_rel_pos.z / rel_distance);
		const float st = math::Sin(theta2);
		const float ct = math::Cos(theta2);
		const float phi2 = (p2_rel_pos.x == 0 && p2_rel_pos.y == 0) ? 0.f : math::Atan2(p2_rel_pos.y, p2_rel_pos.x);
		const float sp = math::Sin(phi2);
		const float cp = math::Cos(phi2);

		// Set p2 velocity as resting
		const math::vec p1_rel_vel = p1.velocity - p2.velocity;

		// Get p1 velocity in a rotated coordinate system where p2 lies on the z-axis
		math::vec p1_rot_vel = {
			ct * cp * p1_rel_vel.x + ct * sp * p1_rel_vel.y - st * p1_rel_vel.z,
			cp * p1_rel_vel.y - sp * p1_rel_vel.x,
			st * cp * p1_rel_vel.x + st * sp * p1_rel_vel.y + ct * p1_rel_vel.z };

		// Get angles and normalized impact parameter
		const float thetav = math::Acos(RE_Math::CapF(p1_rot_vel.z / rel_speed, -1.f, 1.f));
		const float phiv = (p1_rot_vel.x == 0 && p1_rot_vel.y == 0) ? 0.f : math::Atan2(p1_rot_vel.y, p1_rot_vel.x);
		const float dr = rel_distance * math::Sin(thetav) / (p1.col_radius + p2.col_radius);

		if (!(thetav > RE_Math::pi_div2 || math::Abs(dr) > 1))
		{
			// Get time to collision
			const float t = (rel_distance * math::Cos(thetav) - (p1.col_radius + p2.col_radius) * math::Sqrt(1 - dr * dr)) / rel_speed;
			if (t < dt - p1.dt_offset)
			{
				// Move to collision position
				p1.position += p1.velocity * t;
				p2.position += p2.velocity * t;
				p1.dt_offset += t;
				p2.dt_offset += t;

				// Get impact angles
				const float a = math::Tan(thetav + math::Asin(-dr));
				const float sbeta = math::Sin(phiv);
				const float cbeta = math::Cos(phiv);

				const float mass_ratio = p2.mass / p1.mass;

				//  Set velocities and rotate to original coordinate system
				math::vec p2_rot_vel;
				p2_rot_vel.z = 2.f * (p1_rot_vel.z + a * (cbeta * p1_rot_vel.x + sbeta * p1_rot_vel.y)) / ((1.f + a * a) * (1.f + mass_ratio));
				p2_rot_vel.x = a * cbeta * p2_rot_vel.z;
				p2_rot_vel.y = a * sbeta * p2_rot_vel.z;
				p1_rot_vel -= mass_ratio * p2_rot_vel;

				p1.velocity = p2.velocity + math::vec({
					ct * cp * p1_rot_vel.x - sp * p1_rot_vel.y + st * cp * p1_rot_vel.z,
					ct * sp * p1_rot_vel.x + cp * p1_rot_vel.y + st * sp * p1_rot_vel.z,
					ct * p1_rot_vel.z - st * p1_rot_vel.x });

				p2.velocity += math::vec({
					ct * cp * p2_rot_vel.x - sp * p2_rot_vel.y + st * cp * p2_rot_vel.z,
					ct * sp * p2_rot_vel.x + cp * p2_rot_vel.y + st * sp * p2_rot_vel.z,
					ct * p2_rot_vel.z - st * p2_rot_vel.x });

				// Elasticity
				const math::vec momentum = (p1.mass * p1.velocity + p2.mass * p2.velocity) / (p1.mass + p2.mass);
				const float restitution = (p1.col_restitution + p2.col_restitution) / 0.5f;
				p1.velocity = (p1.velocity - momentum) * restitution + momentum;
				p2.velocity = (p2.velocity - momentum) * restitution + momentum;
			}
		}
	}
}
