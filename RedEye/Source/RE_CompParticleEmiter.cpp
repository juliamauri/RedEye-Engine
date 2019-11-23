#include "RE_CompParticleEmiter.h"

#include "Application.h"
#include "RE_ResourceManager.h"
#include "TimeManager.h"
#include "RE_Mesh.h"
#include "RE_TextureImporter.h"
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
}

void RE_CompParticleEmitter::CleanUp()
{
}

void RE_CompParticleEmitter::PreUpdate()
{
}

void RE_CompParticleEmitter::Update()
{
	int spawns_needed = 0;

	if (emissor_life > 0.f)
	{
		time_counter += App->time->GetDeltaTime();

		if (time_counter <= emissor_life)
		{
			spawn_counter += App->time->GetDeltaTime();

			while (spawn_counter > 1000.f / emissionRate)
			{
				spawn_counter -= (1000.f / emissionRate);
				spawns_needed++;
			}
		}
	}
	else
	{
		spawn_counter += App->time->GetDeltaTime();

		while (spawn_counter > 1.f / emissionRate)
		{
			spawn_counter -= (1.f / emissionRate);
			spawns_needed++;
		}
	}

	UpdateParticles(spawns_needed);
}

void RE_CompParticleEmitter::PostUpdate()
{
}

void RE_CompParticleEmitter::OnPlay()
{
	LOG_SECONDARY("particle emitter play");

	if (particles)
		DEL_A(particles);

	particles = new Particle[max_particles = (unsigned int)(emissionRate * (lifetime + lifetime_margin))];

	time_counter = 0.0f;
	spawn_counter = 0.0f;

	for (unsigned int i = 0; i < max_particles; i++)
		particles[i].SetUp(this);
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
	for (unsigned int i = 0; i < max_particles; i++)
	{
		if (particles[i].Alive())
			particles[i].Draw(shader);
	}
}

void RE_CompParticleEmitter::DrawProperties()
{
	if (ImGui::CollapsingHeader("Particle Emitter"))
	{
		ImGui::SliderFloat("Emissor Life", &emissor_life, -1.0f, 10.0f, "%.2f");
		ImGui::Separator();


		ImGui::SliderFloat("Emission Rate", &emissionRate, 0.0f, 20.f, "%.2f");
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
			//((RE_Texture*)App->resources->At(mParticle->ma))->DrawTextureImGui();
		}
	}
}

bool RE_CompParticleEmitter::LocalEmission() const
{
	return local_emission;
}

bool RE_CompParticleEmitter::EmmissionFinished() const
{
	return emissor_life < 0.f ? false : time_counter <= emissor_life;
}

RE_Mesh * RE_CompParticleEmitter::GetMesh() const
{
	return mParticle;
}

//void RE_CompParticleEmitter::Serialize(JSONNode * node, rapidjson::Value * val)
//{
//}

void RE_CompParticleEmitter::SetUp(RE_Mesh * particle, unsigned int shader)
{
	mParticle = particle;
	this->shader = shader;
}

void RE_CompParticleEmitter::ResetParticle(Particle * p)
{
	p->Emit();
}

void RE_CompParticleEmitter::UpdateParticles(int spawns_needed)
{
	Particle* p = nullptr;

	for (unsigned int i = 0; i < max_particles; i++)
	{
		p = &particles[i];

		if (!p->Alive())
		{
			if (spawns_needed > 0)
			{
				ResetParticle(p);
				spawns_needed--;
			}
		}
		else
		{
			p->Update();
		}
	}
}