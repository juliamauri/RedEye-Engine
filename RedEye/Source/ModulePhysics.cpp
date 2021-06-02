#include "ModulePhysics.h"

#include "Application.h"
#include "ModuleScene.h"
#include "RE_PrimitiveManager.h"
#include "RE_Time.h"
#include "RE_CompCamera.h"

ModulePhysics::ModulePhysics() : Module("Physics") {}
ModulePhysics::~ModulePhysics() {}

void ModulePhysics::Update()
{
	const float global_dt = RE_TIME->GetDeltaTime();

	if (use_fixed_dt)
	{
		dt_offset += global_dt;

		float final_dt = 0.f;
		while (dt_offset >= fixed_dt)
		{
			dt_offset -= fixed_dt;
			final_dt += fixed_dt;
		}

		if (final_dt > 0.f)
		{
			update_count++;
			for (auto sim : particles.simulations)
				sim->Update(final_dt);
		}
	}
	else
	{
		update_count++;
		for (auto sim : particles.simulations)
			sim->Update(global_dt);
	}

	time_counter += global_dt;
	if (time_counter >= 1.f)
	{
		time_counter--;
		updates_per_s = update_count;
		update_count = 0.f;
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
		particles.DrawDebug();
	}
}

void ModulePhysics::DrawEditor()
{
	if (ImGui::CollapsingHeader(name))
	{
		ImGui::Text("Updates/s: %.1f", updates_per_s);

		ImGui::Checkbox("Used Fixed Update", &use_fixed_dt);
		if (use_fixed_dt)
		{
			float tmp = 1.f / fixed_dt;
			if (ImGui::DragFloat("Dt", &tmp, 1.f, 1.f, 480.f, "%.0f"))
				fixed_dt = 1.f / tmp;
		}

		particles.DrawEditor();
	}
}

RE_ParticleEmitter* ModulePhysics::AddEmitter()
{
	RE_ParticleEmitter* ret = new RE_ParticleEmitter();

	// Default setup
	ret->initial_lifetime.val = 12.f;
	ret->initial_pos.shape = RE_EmissionShape::Type::CIRCLE;
	ret->initial_pos.geo.circle = math::Circle(math::vec::zero, math::vec(0.f, 1.f, 0.f), 1.f);
	ret->collider.mass.val = 1.f;
	ret->collider.radius.val = 0.5f;
	ret->collider.restitution.val = 0.9f;

	// Curve setup
	ret->curve.push_back({ -1.0f, 0.0f });// init data so editor knows to take it from here
	for (int i = 1; i < ret->total_points; i++)
		ret->curve.push_back({ 0.0f, 0.0f });

	// Prim setup
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
		if (sim->id == emitter_id)
			return sim->particle_count;

	return 0u;
}

eastl::list<RE_Particle*>* ModulePhysics::GetParticles(unsigned int emitter_id) const
{
	for (auto sim : particles.simulations)
		if (sim->id == emitter_id)
			return &sim->particle_pool;

	return nullptr;
}
