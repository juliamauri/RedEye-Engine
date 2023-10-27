#include "RE_ParticleEmitter.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "RE_Math.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleRenderer3D.h"
#include "RE_ParticleManager.h"
#include "RE_PrimitiveManager.h"

#include "RE_CompPrimitive.h"

RE_ParticleEmitter::RE_ParticleEmitter(bool instance_primitive)
{
	if (!instance_primitive) return;

	primCmp = new RE_CompPoint();
	RE_SCENE->primitives->SetUpComponentPrimitive(primCmp);
}

RE_ParticleEmitter::~RE_ParticleEmitter()
{
	if (!primCmp) return;

	primCmp->UnUseResources();
	DEL(primCmp)
}

unsigned int RE_ParticleEmitter::Update(const float global_dt)
{
	RE_PROFILE(RE_ProfiledFunc::Update, RE_ProfiledClass::ParticleEmitter)
	switch (state)
	{
	case RE_ParticleEmitter::PlaybackState::STOPING:
	{
		Reset();
		state = RE_ParticleEmitter::PlaybackState::STOP;
		break;
	}
	case RE_ParticleEmitter::PlaybackState::RESTART:
	{
		Reset();
		state = RE_ParticleEmitter::PlaybackState::PLAY;
		break;
	}
	case RE_ParticleEmitter::PlaybackState::PLAY:
	{
		if (IsTimeValid(global_dt))
		{
			UpdateParticles();
			UpdateSpawn();
		}
		break;
	}
	default: break;
	}

	return particle_count;
}

void RE_ParticleEmitter::Reset()
{
	if (!particle_pool.empty()) particle_pool.clear();
	particle_pool.shrink_to_fit();

	particle_count = 0u;
	max_dist_sq = max_speed_sq = total_time = 
	spawn_interval.time_offset = spawn_mode.time_offset = 0.f;
	spawn_mode.has_started = false;
	bounding_box.minPoint = bounding_box.maxPoint = math::vec::zero;
}

bool RE_ParticleEmitter::IsTimeValid(const float global_dt)
{
	RE_PROFILE(RE_ProfiledFunc::ParticleTiming, RE_ProfiledClass::ParticleEmitter)
	// Check time limitations
	local_dt = global_dt * time_muliplier;
	if (total_time < start_delay)
	{
		if (total_time + local_dt >= start_delay)
		{
			total_time += local_dt;
			local_dt -= start_delay - total_time;
		}
		else return false;
	}
	else total_time += local_dt;

	if (!loop && total_time >= max_time)
	{
		state = RE_ParticleEmitter::PlaybackState::STOPING;
		return false;
	}

	return local_dt > 0.f;
}

void RE_ParticleEmitter::UpdateParticles()
{
	RE_PROFILE(RE_ProfiledFunc::ParticleUpdate, RE_ProfiledClass::ParticleEmitter)

	max_dist_sq = max_speed_sq = 0.f;
	bounding_box.minPoint = bounding_box.maxPoint = parent_pos * !local_space;

	for (unsigned int index = 0u; index < particle_count; ++index)
	{
		// Check if particle is still alive
		bool is_alive = true;
		switch (initial_lifetime.type) {
		case RE_EmissionSingleValue::Type::VALUE: is_alive = (particle_pool[index].lifetime += local_dt) < initial_lifetime.GetValue(); break;
		case RE_EmissionSingleValue::Type::RANGE: is_alive = (particle_pool[index].lifetime += local_dt) < particle_pool[index].max_lifetime; break;
		default: break; }

		// Iterate collisions
		if (is_alive)
		{
			switch (collider.type) {
			case RE_EmissionCollider::Type::POINT:
			{
				if (collider.inter_collisions)
					for (unsigned int next = index + 1u; next < particle_count; ++next)
						ImpulseCollision(particle_pool[index], particle_pool[next]);

				is_alive = boundary.PointCollision(particle_pool[index]);
				break;
			}
			case RE_EmissionCollider::Type::SPHERE:
			{
				if (collider.inter_collisions)
					for (unsigned int next = index + 1u; next < particle_count; ++next)
						ImpulseCollision(particle_pool[index], particle_pool[next], particle_pool[index].col_radius + particle_pool[next].col_radius);

				is_alive = boundary.SphereCollision(particle_pool[index]);
				break;
			}
			default:
			{
				is_alive = boundary.PointCollision(particle_pool[index]);
				break;
			}
			}
		}

		if (is_alive)
		{
			// Update Speed & Position
			particle_pool[index].position += (particle_pool[index].velocity += external_acc.GetAcceleration() * local_dt) * local_dt;

			// Update Control values
			max_dist_sq = RE_Math::Max(max_dist_sq, (particle_pool[index].position - (parent_pos * !local_space)).LengthSq());
			max_speed_sq = RE_Math::Max(max_speed_sq, particle_pool[index].velocity.LengthSq());

			// Broadphase AABB Boundary
			if (RE_ParticleManager::GetBoundingMode() == RE_ParticleManager::BoundingMode::PER_PARTICLE)
			{
				bounding_box.maxPoint = RE_Math::MaxVecValues(particle_pool[index].position, bounding_box.maxPoint);
				bounding_box.minPoint = RE_Math::MinVecValues(particle_pool[index].position, bounding_box.minPoint);
			}
		}
		else // Remove dead particles
		{
			particle_pool.erase(particle_pool.begin() + index);
			index--;
			particle_count--;
		}
	}

	if (RE_ParticleManager::GetBoundingMode() == RE_ParticleManager::BoundingMode::GENERAL)
	{
		switch (boundary.type) {
		case RE_EmissionBoundary::Type::SPHERE: bounding_box.SetFrom(boundary.geo.sphere); break;
		case RE_EmissionBoundary::Type::AABB: bounding_box = boundary.geo.box; break;
		default: bounding_box.SetFromCenterAndSize(parent_pos * !local_space, math::vec(math::SqrtFast(max_dist_sq))); break; }
	}
}

void RE_ParticleEmitter::UpdateSpawn()
{
	RE_PROFILE(RE_ProfiledFunc::ParticleSpawn, RE_ProfiledClass::ParticleEmitter)
	if (!spawn_interval.IsActive(local_dt)) return;

	uint to_add = RE_Math::Min(
		spawn_mode.CountNewParticles(local_dt),
		max_particles - particle_count);

	particle_count += to_add;
	if (particle_pool.capacity() < particle_count)
	{
		uint allocation_step = max_particles / 10u;
		uint desired_capacity = allocation_step;

		while (particle_count < desired_capacity)
			desired_capacity += allocation_step;

		particle_pool.reserve(desired_capacity);
	}

	for (uint i = 0u; i < to_add; ++i)
		particle_pool.push_back(RE_Particle(
			initial_lifetime.GetValue(),
			local_space ? initial_pos.GetPosition() : initial_pos.GetPosition() + parent_pos,
			!inherit_speed ? initial_speed.GetValue() : initial_speed.GetValue() + parent_speed,
			collider.mass.GetValue(), collider.radius.GetValue(), collider.restitution.GetValue(),
			light.GetColor(), light.GetIntensity(), light.GetSpecular()));
}

void RE_ParticleEmitter::ImpulseCollision(RE_Particle& p1, RE_Particle& p2, const float combined_radius) const
{
	RE_PROFILE(RE_ProfiledFunc::ParticleCollision, RE_ProfiledClass::ParticleEmitter)

	// Check particle collision
	const math::vec collision_dir = p1.position - p2.position;
	const float dist2 = collision_dir.Dot(collision_dir);
	if (dist2 > combined_radius * combined_radius) return;

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
	if (dot >= 0.f) return;

	// Resolve applying impulse
	const math::vec impulse = col_normal * (-(p1.col_restitution + p2.col_restitution) * dot) / (p1_inv_mass + p2_inv_mass);
	p1.velocity += impulse * p1_inv_mass;
	p2.velocity -= impulse * p2_inv_mass;
}