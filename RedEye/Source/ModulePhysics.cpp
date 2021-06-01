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
	for (auto sim : particles.simulations)
		sim->Update(global_dt);
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
		particles.DrawDebug();
		glEnd();
	}
}

void ModulePhysics::DrawEditor()
{
	if (ImGui::CollapsingHeader(name))
	{
		int tmp = particles.GetCircleSteps();
		if (ImGui::DragInt("Steps", &tmp, 1.f, 0, 64))
			particles.SetCircleSteps(tmp);
	}
}

RE_ParticleEmitter* ModulePhysics::AddEmitter()
{
	RE_ParticleEmitter* ret = new RE_ParticleEmitter();

	ret->initial_lifetime.val = 2.f;
	ret->initial_pos.shape = RE_EmissionShape::Type::SPHERE;
	ret->initial_pos.geo.sphere = math::Sphere(math::vec::zero, 1.f);
	ret->initial_mass.val = 1.f;
	ret->initial_col_radius.val = 1.f;
	ret->initial_col_restitution.val = 0.9f;

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
