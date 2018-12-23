#include "RE_CompParticleEmiter.h"

#include "RE_Mesh.h"
#include "Texture2DManager.h"
#include "ImGui\imgui.h"


RE_CompParticleEmitter::RE_CompParticleEmitter(RE_GameObject * go) : RE_Component(C_PARTICLEEMITER, go)
{
}

RE_CompParticleEmitter::~RE_CompParticleEmitter()
{
}

void RE_CompParticleEmitter::DrawProperties()
{
	if (ImGui::CollapsingHeader("Particle Emitter"))
	{
		float ps[3] = { point_particle_spawn.x, point_particle_spawn.y, point_particle_spawn.z };
		if (ImGui::DragFloat3("Spawn Point", ps, 0.1f, -10000.f, 10000.f, "%.2f"))
			point_particle_spawn.Set(ps[0], ps[1], ps[2]);

		float dp[3] = { direction_particle_spawn.x, direction_particle_spawn.y, direction_particle_spawn.z };
		if (ImGui::DragFloat3("Spawn Direction", dp, 0.1f, -10000.f, 10000.f, "%.2f"))
			direction_particle_spawn.Set(dp[0], dp[1], dp[2]);

		float gp[3] = { gravity_particle.x, gravity_particle.y, gravity_particle.z };
		if (ImGui::DragFloat3("Gravity Direction", gp, 0.1f, -10000.f, 10000.f, "%.2f"))
			gravity_particle.Set(gp[0], gp[1], gp[2]);

		ImGui::SliderFloat("Angle margin", &angle_margin, 0.0f, 180.0f, "%.2f");
		ImGui::SliderFloat("Duration", &duration, -1.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Lifetime", &lifetime, 0.0f, 10.0f, "%.2f");
		ImGui::SliderFloat("Lifetime Margin", &lifetime_margin, 0.0f, 10.0f, "%.2f");
		ImGui::SliderInt("Emision Rate", &emisionRate, 0, MAX_PARTICLES);

		float sp[3] = { speed_particle.x, speed_particle.y, speed_particle.z };
		if (ImGui::DragFloat3("Speed particle", sp, 0.1f, -10000.f, 10000.f, "%.2f"))
			speed_particle.Set(sp[0], sp[1], sp[2]);
		ImGui::SliderFloat("Speed Margin", &speed_margin, 0.0f, 10.0f, "%.2f");

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
