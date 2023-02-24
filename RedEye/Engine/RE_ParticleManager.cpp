#include "RE_ParticleManager.h"

#include "Application.h"
#include "RE_ResourceManager.h"
#include "ModuleRenderer3D.h"
#include <GL/glew.h>

#if defined(PARTICLE_PHYSICS_TEST) || defined(PARTICLE_RENDER_TEST)
#include "ModulePhysics.h"

#ifdef PARTICLE_RENDER_TEST

RE_Timer ParticleManager::timer_simple(false);

#endif // PARTICLE_RENDER_TEST

#endif // PARTICLE_PHYSICS_TEST || PARTICLE_RENDER_TEST

#include "RE_Math.h"
#include "RE_Profiler.h"
#include "RE_GLCache.h"

#include "RE_InternalResources.h"
#include "RE_ShaderImporter.h"
#include "RE_Shader.h"
#include "RE_Mesh.h"
#include "RE_Particle.h"

#include "RE_CompCamera.h"
#include "RE_CompTransform.h"
#include "RE_CompPrimitive.h"
#include "RE_CompLight.h"

#include <EASTL/vector.h>

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

void ParticleManager::Update(const float dt)
{
	RE_PROFILE(PROF_Update, PROF_ParticleManager);

#if defined(PARTICLE_PHYSICS_TEST) || defined(PARTICLE_RENDER_TEST)

	ProfilingTimer::p_count = particle_count;

#ifdef PARTICLE_PHYSICS_TEST

	RE_Timer timer_simple;

#endif // PARTICLE_PHYSICS_TEST
#endif // PARTICLE_PHYSICS_TEST || PARTICLE_RENDER_TEST

	particle_count = 0u;
	for (auto sim : simulations) particle_count += sim->Update(dt);

#if defined(PARTICLE_PHYSICS_TEST) || defined(PARTICLE_RENDER_TEST)

	if (ProfilingTimer::wait4frame > 0)
	{
		ProfilingTimer::wait4frame--;
	}
	else if (ProfilingTimer::wait4frame == 0)
	{
		ProfilingTimer::update_time = 0u;
		ProfilingTimer::wait4frame--;
		RE_Profiler::Reset();
		RE_Profiler::Start();
		RE_ParticleEmitter::demo_emitter->state = RE_ParticleEmitter::PlaybackState::PLAY;
		RE_PHYSICS->mode = ModulePhysics::UpdateMode::FIXED_TIME_STEP;
	}

#ifdef PARTICLE_PHYSICS_TEST

	else
	{
		ProfilingTimer::update_time = RE_Math::MaxUI(timer_simple.Read(), ProfilingTimer::update_time);
		if (ProfilingTimer::update_time > 33u || RE_ParticleEmitter::demo_emitter->particle_count == RE_ParticleEmitter::demo_emitter->max_particles)
		{
			RE_PHYSICS->mode = ModulePhysics::UpdateMode::ENGINE_PAR;
			RE_Profiler::Deploy(RE_ParticleEmitter::filename.c_str());
			ProfilingTimer::current_sim < 11 ? RE_ParticleEmitter::demo_emitter->DemoSetup() : App->Quit();
		}
	}
#endif // PARTICLE_PHYSICS_TEST
#endif // PARTICLE_PHYSICS_TEST || PARTICLE_RENDER_TEST
}

void ParticleManager::Clear()
{
	simulations.clear();
}

void ParticleManager::DrawSimulation(unsigned int index, math::float3 go_position, math::float3 go_up) const
{
	RE_PROFILE(PROF_DrawParticles, PROF_ParticleManager);

	RE_ParticleEmitter* simulation = nullptr;
	eastl::list<RE_ParticleEmitter*>::const_iterator it;
	for (it = simulations.begin(); it != simulations.end(); ++it)
	{
		if ((*it)->id == index)
		{
			simulation = *it;
			break;
		}
	}
	if(!simulation || !simulation->active_rendering) return;

#ifdef PARTICLE_RENDER_TEST

	if (ProfilingTimer::wait4frame < 0) {
		ProfilingTimer::update_time = RE_Math::MaxUI(timer_simple.Read(), ProfilingTimer::update_time);
		if (ProfilingTimer::update_time > 33u || RE_ParticleEmitter::demo_emitter->particle_count == RE_ParticleEmitter::demo_emitter->max_particles)
		{
			RE_PHYSICS->mode = ModulePhysics::UpdateMode::ENGINE_PAR;

			RE_Profiler::Deploy(RE_ParticleEmitter::filename.c_str());
			ProfilingTimer::current_sim < 3 // MAX SIMULATIONS INDEX
				? RE_ParticleEmitter::demo_emitter->DemoSetup() : App->Quit();
		}

		timer_simple.Stop();
		timer_simple.Start();
	}


#endif // PARTICLE_RENDER_TEST

	// Get Shader and uniforms
	const RE_Shader* pS = static_cast<RE_Shader*>(RE_RES->At(RE_RES->internalResources->GetParticleShader()));
	const unsigned int shader = pS->GetID();
	RE_GLCache::ChangeShader(shader);

	RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);

	// Get Camera Transform
	RE_CompTransform* cT = ModuleRenderer3D::GetCamera()->GetTransform();
	const math::float3 cUp = cT->GetUp().Normalized();

	for (const auto &p : simulation->particle_pool)
	{
		// Calculate Particle Transform
		const math::float3 partcleGlobalpos = simulation->local_space ? go_position + p.position : p.position;
		math::float3 front, right, up;
		switch (simulation->orientation)
		{
		case RE_ParticleEmitter::ParticleDir::FromPS:
		{
			front = (go_position - partcleGlobalpos).Normalized();
			right = front.Cross(go_up).Normalized();
			if (right.x < 0.0f) right *= -1.0f;
			up = right.Cross(front).Normalized();
			break;
		}
		case RE_ParticleEmitter::ParticleDir::Billboard:
		{
			front = cT->GetGlobalPosition() - partcleGlobalpos;
			front.Normalize();
			right = front.Cross(cUp).Normalized();
			if (right.x < 0.0f) right *= -1.0f;
			up = right.Cross(front).Normalized();
			break;
		}
		case RE_ParticleEmitter::ParticleDir::Custom:
		{
			front = simulation->direction.Normalized();
			right = front.Cross(cUp).Normalized();
			if (right.x < 0.0f) right *= -1.0f;
			up = right.Cross(front).Normalized();
			break;
		}
		}

		// Rotate and upload Transform
		math::float4x4 rotm;
		rotm.SetRotatePart(math::float3x3(right, up, front));
		math::vec roteuler = rotm.ToEulerXYZ();
		pS->UploadModel(math::float4x4::FromTRS(partcleGlobalpos, math::Quat::identity * math::Quat::FromEulerXYZ(roteuler.x, roteuler.y, roteuler.z), simulation->scale).Transposed().ptr());

		float weight = 1.f;

		// Lightmode
		if (ModuleRenderer3D::GetLightMode() == LightMode::LIGHT_DEFERRED)
		{
			const bool cNormal = !simulation->meshMD5 && !simulation->primCmp;
			RE_ShaderImporter::setFloat(shader, "customNormal", static_cast<float>(cNormal));
			if (cNormal) RE_ShaderImporter::setFloat(shader, "normal", front);
			RE_ShaderImporter::setFloat(shader, "specular", 2.5f);
			RE_ShaderImporter::setFloat(shader, "shininess", 16.0f);
			RE_ShaderImporter::setFloat(shader, "opacity", 1.0f);
		}
		else
		{
			//Opacity
			switch (simulation->opacity.type) {
			case RE_PR_Opacity::Type::OVERLIFETIME: weight = p.lifetime / simulation->initial_lifetime.GetMax(); break;
			case RE_PR_Opacity::Type::OVERDISTANCE: weight = math::SqrtFast(p.position.LengthSq()) / math::SqrtFast(simulation->max_dist_sq); break;
			case RE_PR_Opacity::Type::OVERSPEED: weight = math::SqrtFast(p.velocity.LengthSq()) / math::SqrtFast(simulation->max_speed_sq); break;
			default: break;
			}

			RE_ShaderImporter::setFloat(shader, "opacity", simulation->opacity.GetValue(weight));
		}

		// Color
		switch (simulation->color.type) {
		case RE_PR_Color::Type::OVERLIFETIME: weight = p.lifetime / simulation->initial_lifetime.GetMax(); break;
		case RE_PR_Color::Type::OVERDISTANCE: weight = math::SqrtFast(p.position.LengthSq()) / math::SqrtFast(simulation->max_dist_sq); break;
		case RE_PR_Color::Type::OVERSPEED: weight = math::SqrtFast(p.velocity.LengthSq()) / math::SqrtFast(simulation->max_speed_sq); break;
		default: break;
		}

		RE_ShaderImporter::setFloat(shader, "cdiffuse", simulation->color.GetValue(weight));

		// Draw Call
		if (simulation->meshMD5) dynamic_cast<RE_Mesh*>(RE_RES->At(simulation->meshMD5))->DrawMesh(shader);
		else simulation->primCmp->SimpleDraw();
	}

#ifdef PARTICLE_RENDER_TEST

	timer_simple.Pause();

#endif // PARTICLE_RENDER_TEST
}

void ParticleManager::CallLightShaderUniforms(unsigned int index, math::float3 go_position, unsigned int shader, const char* array_unif_name, unsigned int& count, unsigned int maxLights, bool sharedLight) const
{
	RE_PROFILE(PROF_DrawParticlesLight, PROF_ParticleManager);

	RE_ParticleEmitter* simulation = nullptr;
	eastl::list<RE_ParticleEmitter*>::const_iterator it;
	for (it = simulations.begin(); it != simulations.end(); ++it)
	{
		if ((*it)->id == index)
		{
			simulation = *it;
			break;
		}
	}
	if (!simulation) return;

	eastl::string array_name(array_unif_name);
	array_name += "[";
	eastl::string unif_name;

	if (!sharedLight)
	{
		eastl::string uniform_name("pInfo.tclq");
		RE_ShaderImporter::setFloat(
			RE_ShaderImporter::getLocation(shader, (uniform_name).c_str()),
			static_cast<float>(L_POINT),
			simulation->light.constant,
			simulation->light.linear,
			simulation->light.quadratic);

		for (const auto p : simulation->particle_pool)
		{
			if (count == maxLights) return;
			unif_name = array_name + eastl::to_string(count++) + "].";

			const math::float3 p_global_pos = simulation->local_space ? go_position + p.position : p.position;
			switch (simulation->light.type) {
			case RE_PR_Light::Type::UNIQUE:
			{
				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "diffuseSpecular").c_str()),
					simulation->light.color.x, simulation->light.color.y, simulation->light.color.z, simulation->light.specular);

				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "positionIntensity").c_str()),
					p_global_pos.x, p_global_pos.y, p_global_pos.z, simulation->light.intensity);

				break;
			}
			case RE_PR_Light::Type::PER_PARTICLE:
			{
				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "diffuseSpecular").c_str()),
					p.lightColor.x, p.lightColor.y, p.lightColor.z, p.specular);

				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "positionIntensity").c_str()),
					p_global_pos.x, p_global_pos.y, p_global_pos.z, p.intensity);

				break;
			}
			default: break;
			}
		}
	}
	else
	{
		const math::vec color = simulation->light.GetColor();
		const float intensity = simulation->light.GetIntensity();

		for (const auto p : simulation->particle_pool)
		{
			if (count == maxLights) return;

			unif_name = array_name + eastl::to_string(count++) + "].";

			switch (simulation->light.type) {
			case RE_PR_Light::Type::UNIQUE:
			{
				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "directionIntensity").c_str()),
					0.f, 0.f, 0.f, intensity);

				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "diffuseSpecular").c_str()),
					color.x, color.y, color.z, simulation->light.GetSpecular());
				break;
			}
			case RE_PR_Light::Type::PER_PARTICLE:
			{
				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "directionIntensity").c_str()),
					0.f, 0.f, 0.f, p.intensity);

				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "diffuseSpecular").c_str()),
					p.lightColor.x, p.lightColor.y, p.lightColor.z, p.specular);
				break;
			}
			default: break;
			}

			const math::float3 partcleGlobalpos = go_position + p.position;
			RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "positionType").c_str()), partcleGlobalpos.x, partcleGlobalpos.y, partcleGlobalpos.z, static_cast<float>(L_POINT));

			const math::vec quadratic = simulation->light.GetQuadraticValues();
			RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "clq").c_str()), quadratic.x, quadratic.y, quadratic.z, 0.f);
		}
	}
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
	ImGui::Text("Total Emitters: %i", simulations.size());
	ImGui::Text("Total Particles: %i", particle_count);

	int tmp = static_cast<int>(RE_ParticleEmitter::mode);
	if (ImGui::Combo("AABB Enclosing", &tmp, "General\0Per Particle\0"))
		RE_ParticleEmitter::mode = static_cast<RE_ParticleEmitter::BoundingMode>(tmp);

	ImGui::DragFloat("Point size", &point_size, 1.f, 0.f, 100.f);

	tmp = static_cast<int>(circle_steps);
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
	RE_PROFILE(PROF_Update, PROF_ModulePhysics);

	const float interval = RE_Math::pi_x2 / circle_steps;
	for (const auto sim : simulations)
	{
		DebugDrawSimulation(sim, interval);
	}
}

void ParticleManager::DebugDrawSimulation(const RE_ParticleEmitter* const sim, const float interval) const
{
	if (sim->initial_pos.type || sim->boundary.type)
	{
		glBegin(GL_LINES);

		// Render Spawn Shape
		glColor4f(1.0f, 0.27f, 0.f, 1.f); // orange
		switch (sim->initial_pos.type) {
		case RE_EmissionShape::Type::CIRCLE:
		{
			math::Circle c = sim->initial_pos.geo.circle;
			c.pos += sim->parent_pos;
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
			c.pos += sim->parent_pos;
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
				glVertex3fv((sim->initial_pos.geo.box.Edge(i).a + sim->parent_pos).ptr());
				glVertex3fv((sim->initial_pos.geo.box.Edge(i).b + sim->parent_pos).ptr());
			}

			break;
		}
		case RE_EmissionShape::Type::SPHERE:
		{
			DrawAASphere(sim->parent_pos + sim->initial_pos.geo.sphere.pos, sim->initial_pos.geo.sphere.r);
			break;
		}
		case RE_EmissionShape::Type::HOLLOW_SPHERE:
		{
			DrawAASphere(sim->parent_pos + sim->initial_pos.geo.hollow_sphere.first.pos, sim->initial_pos.geo.hollow_sphere.first.r - sim->initial_pos.geo.hollow_sphere.second);
			DrawAASphere(sim->parent_pos + sim->initial_pos.geo.hollow_sphere.first.pos, sim->initial_pos.geo.hollow_sphere.first.r + sim->initial_pos.geo.hollow_sphere.second);
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
				const math::Circle c = sim->boundary.geo.plane.GenerateCircle(sim->parent_pos, j * j);
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
			DrawAASphere(sim->parent_pos + sim->boundary.geo.sphere.pos, sim->boundary.geo.sphere.r);
			break;
		}
		case RE_EmissionBoundary::AABB:
		{
			for (int i = 0; i < 12; i++)
			{
				glVertex3fv((sim->parent_pos + sim->boundary.geo.box.Edge(i).a).ptr());
				glVertex3fv((sim->parent_pos + sim->boundary.geo.box.Edge(i).b).ptr());
			}

			break;
		}
		default: break;
		}

		// Render Shape Collider
		if (sim->collider.type == RE_EmissionCollider::Type::SPHERE)
		{
			glColor4f(0.1f, 0.8f, 0.1f, 1.f); // light green
			for (const auto p : sim->particle_pool)
				DrawAASphere(sim->local_space ? sim->parent_pos + p.position : p.position, p.col_radius);
		}

		glEnd();
	}

	// Render Point Collider
	if (sim->collider.type == RE_EmissionCollider::Type::POINT)
	{
		glPointSize(point_size);
		glBegin(GL_POINTS);
		glColor4f(0.1f, 0.8f, 0.1f, 1.f); // light green

		if (sim->local_space)
			for (const auto p : sim->particle_pool)
				glVertex3fv((sim->parent_pos + p.position).ptr());
		else
			for (const auto p : sim->particle_pool)
				glVertex3fv(p.position.ptr());

		glPointSize(1.f);
		glEnd();
	}
}

float ParticleManager::GetInterval() const
{
	return RE_Math::pi_x2 / circle_steps;
}

bool ParticleManager::SetEmitterState(unsigned int index, RE_ParticleEmitter::PlaybackState state)
{
	eastl::list<eastl::pair<RE_ParticleEmitter*, eastl::list<RE_Particle*>*>*>::iterator it;
	for (const auto sim : simulations)
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
