#include "ModulePhysics.h"

#include "Application.h"
#include "ModuleScene.h"
#include "RE_PrimitiveManager.h"
#include "RE_Time.h"
#include "RE_Math.h"
#include "RE_Particle.h"
#include "RE_CompCamera.h"

ModulePhysics::ModulePhysics() : Module("Physics") {

}
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
	const float global_dt = RE_TIME->GetDeltaTime();

	for (auto sim : particles.simulations)
	{
		switch (sim->first->state)
		{
		case RE_ParticleEmitter::STOP:
		{
			if (!sim->second->empty())
				sim->second->clear();
			break;
		}
		case RE_ParticleEmitter::PLAY:
		{
			// Spawn new particles
			float local_dt = global_dt * sim->first->speed_muliplier;
			const float spawn_period = 1.f / sim->first->spawn_frequency;
			int to_add = static_cast<int>((sim->first->spawn_offset += local_dt) * sim->first->spawn_frequency);
			sim->first->spawn_offset -= static_cast<float>(to_add) / sim->first->spawn_frequency;
			for (int i = 0; i < to_add; ++i)
			{
				RE_Particle* particle = new RE_Particle();

				// Set base properties
				particle->lifetime = 0.f;
				particle->max_lifetime = sim->first->v_lifetime.GetValue();
				particle->dt_offset = local_dt - sim->first->spawn_offset - (spawn_period * (i + 1));
				
				particle->position = sim->first->v_pos.GetPosition();
				particle->velocity = sim->first->v_speed.GetSpeed();

				// Set physic properties
				particle->mass = sim->first->v_mass.GetValue();
				particle->col_radius = sim->first->v_col_radius.GetValue();
				particle->col_restitution = sim->first->v_col_restitution.GetValue();

				// Set light properties
				particle->intensity = (sim->first->randomIntensity) ? RE_MATH->RandomF(sim->first->iClamp[0], sim->first->iClamp[1]) : sim->first->intensity;
				particle->specular = (sim->first->randomSpecular) ? RE_MATH->RandomF(sim->first->sClamp[0], sim->first->sClamp[1]) : sim->first->specular;
				
				if (sim->first->randomLightColor)
					particle->lightColor.Set(RE_MATH->RandomF(), RE_MATH->RandomF(), RE_MATH->RandomF());
				else
					particle->lightColor = sim->first->lightColor;

				// Append particle
				sim->second->push_back(particle);
			}

			// Update particles
			for (eastl::list<RE_Particle*>::iterator p1 = sim->second->begin(); p1 != sim->second->end();)
			{
				// Check if particle is still alive
				bool is_alive = true;
				switch (sim->first->v_lifetime.type) {
				case RE_EmissionSingleValue::Type::VALUE: is_alive = ((*p1)->lifetime += local_dt) < sim->first->v_lifetime.GetValue(); break;
				case RE_EmissionSingleValue::Type::RANGE: is_alive = ((*p1)->lifetime += local_dt) < (*p1)->max_lifetime; break;
				default: break; }

				if (is_alive)
				{
					// Iterate for collisions (other particles could be dead, but dt should always be too small to notice)
					for (eastl::list<RE_Particle*>::iterator p2 = p1.next(); p2 != sim->second->end(); ++p2)
					{
						switch (method) {
						case ModulePhysics::SIMPLE: ImpulseCollision(sim->first, **p1, **p2); break;
						case ModulePhysics::Thomas_Smid: ImpulseCollisionTS(sim->first, **p1, **p2, local_dt); break; }
					}

					// Acceleration
					(*p1)->velocity += sim->first->external_acc.GetAcceleration() * (local_dt -= (*p1)->dt_offset);
					(*p1)->dt_offset = 0.f;

					// Check boundary collision
					if (sim->first->boundary.ParticleCollision(**p1))
					{
						// Cap speed
						if ((*p1)->velocity.LengthSq() > sim->first->maxSpeed * sim->first->maxSpeed)
							(*p1)->velocity = (*p1)->velocity.Normalized() * sim->first->maxSpeed;

						// Update position
						(*p1)->position += (*p1)->velocity * local_dt;

						++p1;
					}
					else
					{
						// Remove dead particles
						DEL(*p1);
						p1 = sim->second->erase(p1);
					}

				}
				else
				{
					// Remove dead particles
					DEL(*p1);
					p1 = sim->second->erase(p1);
				}
			}

			break;
		}
		}
	}
}

void ModulePhysics::CleanUp()
{
	particles.simulations.clear();
}

void ModulePhysics::DrawDebug(RE_CompCamera* current_camera) const
{
	if (!particles.simulations.empty())
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(current_camera->GetProjectionPtr());
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf((current_camera->GetView()).ptr());
		glBegin(GL_LINES);
		glColor4f(debug_color[0], debug_color[1], debug_color[2], debug_color[3]);
		particles.DrawDebug();
		glEnd();
	}
}

void ModulePhysics::DrawEditor()
{
	if (ImGui::CollapsingHeader(name))
	{
		int tmp = static_cast<int>(method);
		if (ImGui::Combo("Collision Resolution Method", &tmp, "Simple\0Thomas Smid\0"))
			method = static_cast<CollisionResolution>(tmp);

		tmp = particles.GetCircleSteps();
		if (ImGui::DragInt("Steps", &tmp, 1.f, 0, 64))
			particles.SetCircleSteps(tmp);

		ImGui::ColorEdit4("Debug Color", debug_color);
	}
}

RE_ParticleEmitter* ModulePhysics::AddEmitter()
{
	RE_ParticleEmitter* ret = new RE_ParticleEmitter();

	ret->v_lifetime.val = 10.f;
	ret->v_pos.geo.point = { 0.f, 5.f, 0.f };
	ret->v_mass.val = 1.f;
	ret->v_col_radius.val = 1.f;
	ret->v_col_restitution.val = 0.9f;

	// Curve SetUp
	ret->curve.push_back({ -1.0f, 0.0f });// init data so editor knows to take it from here
	for (int i = 1; i < ret->total_points; i++)
		ret->curve.push_back({ 0.0f, 0.0f });

	// Prim SetUp
	ret->primCmp = new RE_CompPoint();
	RE_SCENE->primitives->SetUpComponentPrimitive(ret->primCmp);

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

eastl::list<RE_Particle*>* ModulePhysics::GetParticles(unsigned int emitter_id) const
{
	for (auto sim : particles.simulations)
	{
		if (sim->first->id == emitter_id)
		{
			return sim->second;
		}
	}
	return nullptr;
}

void ModulePhysics::ImpulseCollision(RE_ParticleEmitter* emitter, RE_Particle& p1, RE_Particle& p2) const
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

void ModulePhysics::ImpulseCollisionTS(RE_ParticleEmitter* emitter, RE_Particle& p1, RE_Particle& p2, const float dt) const
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
		const float thetav = math::Acos(RE_Math::Cap(p1_rot_vel.z / rel_speed, -1.f, 1.f));
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

