#include "RE_CompParticleEmiter.h"

#include "Application.h"
#include "TimeManager.h"
#include "RE_Mesh.h"
#include "Texture2DManager.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "ImGui\imgui.h"
#include "OutputLog.h"


RE_CompParticleEmitter::RE_CompParticleEmitter(RE_GameObject * go) : RE_Component(C_PARTICLEEMITER, go)
{
}

RE_CompParticleEmitter::~RE_CompParticleEmitter()
{
}

void RE_CompParticleEmitter::Init()
{
	particles = new Particle[max_particles = (unsigned int)(emissionRate * (lifetime + lifetime_margin))];
	timer_duration = emissor_life;

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
			if (emisionCount <= emissionRate)
			{
				if (!p->isEmitted())
				{
					
					/*direction_particle.x += speed_margin.x * App->math->RandomF(-1.f, 1.f);
					direction_particle.y += speed_margin.y * App->math->RandomF(-1.f, 1.f);
					direction_particle.z += speed_margin.z * App->math->RandomF(-1.f, 1.f);*/

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

void RE_CompParticleEmitter::OnPlay()
{
	LOG_SECONDARY("particle emitter play");
}

void RE_CompParticleEmitter::OnPause()
{

}

void RE_CompParticleEmitter::OnStop()
{
	LOG_SECONDARY("particle emitter stop");
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
		ImGui::SliderFloat("Emissor Life", &emissor_life, -1.0f, 10.0f, "%.2f");
		ImGui::Separator();


		ImGui::SliderFloat("Emission Rate", &emissionRate, 0.0f, 20.f, "%.2f");
		ImGui::SliderInt("Max Particles", &max_particles, 0, MAX_PARTICLES);
		float ps[3] = { spawn_position_offset.x, spawn_position_offset.y, spawn_position_offset.z };
		if (ImGui::DragFloat3("Spawn Position Offset", ps, 0.1f, -10000.f, 10000.f, "%.2f"))
			spawn_position_offset.Set(ps[0], ps[1], ps[2]);
		float gp[3] = { gravity.x, gravity.y, gravity.z };
		if (ImGui::DragFloat3("Gravity", gp, 0.1f, -10000.f, 10000.f, "%.2f"))
			gravity.Set(gp[0], gp[1], gp[2]);
		ImGui::Checkbox("Local Emission", &local_emission);
		ImGui::Separator();

		ImGui::SliderFloat("Lifetime", &lifetime, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Initial Speed", &initial_speed, 0.0f, 10.0f, "%.2f");
		ImGui::Separator();

		float am[3] = { math::RadToDeg(direction_margin.x), math::RadToDeg(direction_margin.y), math::RadToDeg(direction_margin.z) };
		if (ImGui::DragFloat3("Direction Margin", am, 0.1f, 0.f, 180.f, "%.2f"))
			direction_margin.Set(math::DegToRad(am[0]), math::DegToRad(am[1]), math::DegToRad(am[2]));
		ImGui::SliderFloat("Speed Margin", &speed_margin, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Lifetime Margin", &lifetime_margin, 0.0f, 10.0f, "%.2f");
		ImGui::Separator();

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

bool RE_CompParticleEmitter::LocalEmission() const
{
	return local_emission;
}

void RE_CompParticleEmitter::Serialize(JSONNode * node, rapidjson::Value * val)
{
}
