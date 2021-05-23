#include "ModulePhysics.h"

#include "Application.h"
#include "RE_Time.h"
#include "RE_Math.h"
#include "RE_Particle.h"
#include "RE_CompCamera.h"

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
			const float local_dt = global_dt * sim->first->speed_muliplier;
			const float spawn_period = 1.f / sim->first->spawn_frequency;
			int to_add = sim->first->GetNewSpawns(local_dt);

			// Spawn new particles
			for (int i = 0; i < to_add; ++i)
			{
				RE_Particle* particle = new RE_Particle();

				// Set base properties
				particle->dt_offset = local_dt - sim->first->spawn_offset - (spawn_period * (i + 1));
				particle->position = { 0.f, 0.f, 0.f };

				// Set physic properties
				particle->mass = 1.f;
				particle->velocity = { (RE_MATH->RandomF() * 3.f), 5.f, (RE_MATH->RandomF() * 3.f) };
				particle->col_radius = 0.2f;

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
				(*p1)->lifetime += local_dt;
				if ((*p1)->lifetime < sim->first->lifetime)
				{
					unsigned int collision_count = 0u;

					// Iterate for collisions (other particles could be dead, but dt should always be too small to notice)
					for (eastl::list<RE_Particle*>::iterator p2 = p1.next(); p2 != sim->second->end(); ++p2)
					{
						// Get relative distance & speed
						float rel_distance = ((*p2)->position - (*p1)->position).Length();
						float rel_speed = ((*p2)->velocity - (*p1)->velocity).Length();

						// Check particle collision
						if (rel_distance / 1.2f <= (*p1)->col_radius + (*p2)->col_radius
							&& rel_speed > 0.000001f)
						{
							// Set p1 position as origin
							math::vec p2_rel_pos = (*p2)->position - (*p1)->position;

							// Get polar coordinates
							const float theta2 = acos(p2_rel_pos.z / rel_distance);
							const float st = sin(theta2);
							const float ct = cos(theta2);
							const float phi2 = (p2_rel_pos.x == 0 && p2_rel_pos.y == 0) ? 0.f : atan2(p2_rel_pos.y, p2_rel_pos.x);
							const float sp = sin(phi2);
							const float cp = cos(phi2);

							// Set p2 velocity as resting
							const math::vec p1_rel_vel = (*p1)->velocity - (*p2)->velocity;

							// Get p1 velocity in a rotated coordinate system where p2 lies on the z-axis
							math::vec p1_rot_vel = {
								ct * cp * p1_rel_vel.x + ct * sp * p1_rel_vel.y - st * p1_rel_vel.z,
								cp * p1_rel_vel.y - sp * p1_rel_vel.x,
								st * cp * p1_rel_vel.x + st * sp * p1_rel_vel.y + ct * p1_rel_vel.z };

							// Get angles and normalized impact parameter
							const float thetav = acos(RE_Math::Cap(p1_rot_vel.z / rel_speed, -1.f, 1.f));
							const float phiv = (p1_rot_vel.x == 0 && p1_rot_vel.y == 0) ? 0.f : atan2(p1_rot_vel.y, p1_rot_vel.x);
							const float dr = rel_distance * sin(thetav) / ((*p1)->col_radius + (*p2)->col_radius);

							if (!(thetav > 1.57079632f || fabs(dr) > 1))
							{
								// Get time to collision
								const float t = (rel_distance * cos(thetav) - ((*p1)->col_radius + (*p2)->col_radius) * sqrt(1 - dr * dr)) / rel_speed;
								if (t < local_dt - (*p1)->dt_offset)
								{
									// Move to collision position
									(*p1)->position += (*p1)->velocity * t;
									(*p2)->position += (*p2)->velocity * t;
									(*p1)->dt_offset += t;
									(*p2)->dt_offset += t;

									// Get impact angles
									const float a = tan(thetav + asin(-dr));
									const float sbeta = sin(phiv);
									const float cbeta = cos(phiv);

									const float mass_ratio = (*p2)->mass / (*p1)->mass;

									//  Set velocities and rotate to original coordinate system
									math::vec p2_rot_vel;
									p2_rot_vel.z = 2.f * (p1_rot_vel.z + a * (cbeta * p1_rot_vel.x + sbeta * p1_rot_vel.y)) / ((1.f + a * a) * (1.f + mass_ratio));
									p2_rot_vel.x = a * cbeta * p2_rot_vel.z;
									p2_rot_vel.y = a * sbeta * p2_rot_vel.z;
									p1_rot_vel -= mass_ratio * p2_rot_vel;

									(*p1)->velocity = (*p2)->velocity + math::vec({
										ct * cp * p1_rot_vel.x - sp * p1_rot_vel.y + st * cp * p1_rot_vel.z,
										ct * sp * p1_rot_vel.x + cp * p1_rot_vel.y + st * sp * p1_rot_vel.z,
										ct * p1_rot_vel.z - st * p1_rot_vel.x });

									(*p2)->velocity += math::vec({
										ct * cp * p2_rot_vel.x - sp * p2_rot_vel.y + st * cp * p2_rot_vel.z,
										ct * sp * p2_rot_vel.x + cp * p2_rot_vel.y + st * sp * p2_rot_vel.z,
										ct * p2_rot_vel.z - st * p2_rot_vel.x });

									// Elasticity
									const math::vec momentum = ((*p1)->mass * (*p1)->velocity + (*p2)->mass * (*p2)->velocity) / ((*p1)->mass + (*p2)->mass);
									(*p1)->velocity = ((*p1)->velocity - momentum) * sim->first->restitution + momentum;
									(*p2)->velocity = ((*p2)->velocity - momentum) * sim->first->restitution + momentum;

									collision_count++;
								}
							}
						}
					}

					// TODO: Check boundary collisions

					// Acceleration
					const float final_dt = local_dt - (*p1)->dt_offset;
					(*p1)->velocity += sim->first->wind * final_dt;
					(*p1)->velocity.y += sim->first->gravity * final_dt;

					// TODO: Cap speed
					if ((*p1)->velocity.Length() > sim->first->maxSpeed)
						(*p1)->velocity = (*p1)->velocity.Normalized() * sim->first->maxSpeed;

					// Update position
					(*p1)->position += (*p1)->velocity * final_dt;
					(*p1)->dt_offset = 0.f;

					++p1;
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
		particles.DrawDebug(circle_steps);
		glEnd();
	}
}

void ModulePhysics::DrawEditor()
{
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