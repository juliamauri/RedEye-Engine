#include "RE_CompParticleEmiter.h"

#include "Application.h"
#include "TimeManager.h"
#include "RE_Mesh.h"
#include "Texture2DManager.h"
#include "ImGui\imgui.h"


RE_CompParticleEmitter::RE_CompParticleEmitter(RE_GameObject * go) : RE_Component(C_PARTICLEEMITER, go)
{
}

RE_CompParticleEmitter::~RE_CompParticleEmitter()
{
}

void RE_CompParticleEmitter::Init()
{
	particles = new Particle[max_particles = (unsigned int)(emisionRate * (lifetime + lifetime_margin))];
	timer_duration = duration;

	for (unsigned int i = 0; i < max_particles; i++)
		particles[i].SetUp(this, mParticle);
}

void RE_CompParticleEmitter::CleanUp()
{
}

void RE_CompParticleEmitter::PreUpdate()
{
}

void RE_CompParticleEmitter::Update()
{
	if (timer_duration != -1)
		timer_duration -= App->time->GetDeltaTime();

	if (timer_duration > 0 || timer_duration == -1)
	{
		Particle* p = nullptr;
		int emisionCount = 0;
		for (unsigned int i = 0; i < max_particles; i++)
		{
			p = &particles[i];
			if (emisionCount <= emisionRate)
			{
				if (!p->isEmitted())
				{
					
					direction_particle.x += speed_margin.x * App->math->RandomF(-1.f, 1.f);
					direction_particle.y += speed_margin.y * App->math->RandomF(-1.f, 1.f);
					direction_particle.z += speed_margin.z * App->math->RandomF(-1.f, 1.f);

					//direction_particle.

					//p->Emit();
					emisionCount++;
				}
			}
			if (p->isEmitted())
				p->Update();
		}
	}
}

void RE_CompParticleEmitter::PostUpdate()
{
}

void RE_CompParticleEmitter::Draw()
{
	if (timer_duration > 0 || timer_duration == -1)
	{
		for (unsigned int i = 0; i < max_particles; i++)
		{
			if (particles[i].isEmitted())
				particles[i].Draw(shader);
		}
	}
}

void RE_CompParticleEmitter::DrawProperties()
{
	if (ImGui::CollapsingHeader("Particle Emitter"))
	{
		float ps[3] = { point_particle_spawn.x, point_particle_spawn.y, point_particle_spawn.z };
		if (ImGui::DragFloat3("Spawn Point", ps, 0.1f, -10000.f, 10000.f, "%.2f"))
			point_particle_spawn.Set(ps[0], ps[1], ps[2]);

		float gp[3] = { gravity_particle.x, gravity_particle.y, gravity_particle.z };
		if (ImGui::DragFloat3("Gravity Direction", gp, 0.1f, -10000.f, 10000.f, "%.2f"))
			gravity_particle.Set(gp[0], gp[1], gp[2]);

		float am[3] = { math::RadToDeg(angle_margin.x), math::RadToDeg(angle_margin.y), math::RadToDeg(angle_margin.z) };
		if (ImGui::DragFloat3("Gravity Direction", am, 0.1f, 0.f, 180.f, "%.2f"))
			angle_margin.Set(math::DegToRad(am[0]), math::DegToRad(am[1]), math::DegToRad(am[2]));

		ImGui::SliderFloat("Duration", &duration, -1.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Lifetime", &lifetime, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Lifetime Margin", &lifetime_margin, 0.0f, 10.0f, "%.2f");
		ImGui::SliderInt("Emision Rate", &emisionRate, 0, MAX_PARTICLES);

		float sp[3] = { direction_particle.x, direction_particle.y, direction_particle.z };
		if (ImGui::DragFloat3("Speed particle", sp, 0.1f, -10000.f, 10000.f, "%.2f"))
			direction_particle.Set(sp[0], sp[1], sp[2]);
		float smp[3] = { speed_margin.x, speed_margin.y, speed_margin.z };
		if (ImGui::DragFloat3("Speed particle", smp, 0.1f, -10000.f, 10000.f, "%.2f"))
			speed_margin.Set(smp[0], smp[1], smp[2]);

		float rgba[3] = { rgb_alpha.x, rgb_alpha.y, rgb_alpha.z };
		if (ImGui::DragFloat3("RGB Alpha", rgba, 0.1f, 0.0f, 255.0f, "%.2f"))
			rgb_alpha.Set(rgba[0], rgba[1], rgba[2]);

		if (mParticle)
		{
			ImGui::Text("Particle Texture");
			mParticle->textures.at(0).ptr->DrawTextureImGui();
		}
	}
}

void RE_CompParticleEmitter::Serialize(JSONNode * node, rapidjson::Value * val)
{
}
