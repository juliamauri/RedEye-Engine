#include "RE_CompParticleEmiter.h"

#include "Application.h"
#include "ModulePhysics.h"
#include "ModuleRenderer3D.h"
#include "RE_CameraManager.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_Mesh.h"
#include "RE_Shader.h"
#include "RE_GLCache.h"

#include "ModuleScene.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"

#include "ImGui\imgui.h"

RE_CompParticleEmitter::RE_CompParticleEmitter() : RE_Component(C_PARTICLEEMITER) {
}

RE_CompParticleEmitter::~RE_CompParticleEmitter()
{
	if (simulation != nullptr)
		RE_PHYSICS->RemoveEmitter(simulation);
}

void RE_CompParticleEmitter::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent)
{}

void RE_CompParticleEmitter::Update()
{
}

void RE_CompParticleEmitter::Draw() const
{
	if (draw) {
		RE_Shader* pS = static_cast<RE_Shader*>(RE_RES->At(RE_RES->internalResources->GetParticleShader()));

		eastl::list<RE_Particle*>* particles = RE_PHYSICS->GetParticles(simulation->id);
		unsigned int shader = pS->GetID();
		RE_GLCache::ChangeShader(shader);
		RE_GLCache::ChangeVAO(VAO);

		
		RE_CompTransform* transform = static_cast<RE_CompTransform*>(pool_gos->AtCPtr(go)->GetCompPtr(C_TRANSFORM));
		math::float3 goPosition = transform->GetGlobalPosition();
		RE_CompCamera* c = ModuleRenderer3D::GetCamera();
		
		RE_CompTransform* cT = c->GetTransform();


		math::float3 scale = {0.1f,0.1f,0.1f};

		if (!scale.IsFinite())
		{
			scale = math::float3::one;
		}

		for (auto p : *particles) {
			math::float3 partcleGlobalpos = goPosition + p->position;

			math::float3 front = cT->GetGlobalPosition() - partcleGlobalpos;
			front.Normalize();

			math::float3 right = front.Cross(cT->GetUp().Normalized()).Normalized();
			if (right.x < 0.0f) right *= -1.0f;

			math::float3 up = right.Cross(front).Normalized();

			math::float4x4 rotm;
			rotm.SetRotatePart(math::float3x3(right, up, front));
			math::float3 roteuler = rotm.ToEulerXYZ();
			math::float4x4 pMatrix = math::float4x4::FromTRS(partcleGlobalpos, math::Quat::identity * math::Quat::FromEulerXYZ(roteuler.x, roteuler.y, roteuler.z), scale).Transposed();
			pS->UploadModel(pMatrix.ptr());

			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
		}
	}
}

void RE_CompParticleEmitter::DrawProperties()
{
	if (simulation == nullptr)
		simulation = RE_PHYSICS->AddEmitter();

	if (ImGui::CollapsingHeader("Particle Emitter"))
	{
		if (simulation != nullptr)
		{
			if (!draw && ImGui::Button("Start Draw")) {
				glGenVertexArrays(1, &VAO);
				glGenBuffers(1, &VBO);
				glGenBuffers(1, &EBO);

				RE_GLCache::ChangeVAO(VAO);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);

				float triangle[9]{
					-0.5f, -0.5f, 0.0f,
					0.5f, -0.5f, 0.0f,
					0.0f,  0.5f, 0.0f
				};

				unsigned int index[3] = { 0, 1, 2 };

				glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), &triangle[0], GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned int), &index[0], GL_STATIC_DRAW);

				// vertex positions
				int accumulativeOffset = 0;
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
				accumulativeOffset += sizeof(float) * 3;

				RE_GLCache::ChangeVAO(0);

				draw = true;
			}


			switch (simulation->state)
			{
			case RE_ParticleEmitter::STOP:
			{
				if (ImGui::Button("Play")) simulation->state = RE_ParticleEmitter::PLAY;
				break;
			}
			case RE_ParticleEmitter::PLAY:
			{
				if (ImGui::Button("Pause")) simulation->state = RE_ParticleEmitter::PAUSE;
				if (ImGui::Button("Stop")) simulation->state = RE_ParticleEmitter::STOP;
				break;
			}
			case RE_ParticleEmitter::PAUSE:
			{
				if (ImGui::Button("Resume")) simulation->state = RE_ParticleEmitter::PLAY;
				if (ImGui::Button("Stop")) simulation->state = RE_ParticleEmitter::STOP;
				break;
			}
			}

			ImGui::Text("Current particles: %i", RE_PHYSICS->GetParticleCount(simulation->id));
		}
		else
		{
			ImGui::Text("Unregistered simulation");
		}

		ImGui::Separator();

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

		//if (mParticle)
		//{
		//	ImGui::Text("Particle Texture");
		//	//((RE_Texture*)App->resources->At(mParticle->ma))->DrawTextureImGui();
		//}
	}
}

void RE_CompParticleEmitter::SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const
{
}

void RE_CompParticleEmitter::DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources)
{
}

unsigned int RE_CompParticleEmitter::GetBinarySize() const
{
	return 0;
}

void RE_CompParticleEmitter::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const
{
}

void RE_CompParticleEmitter::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
}


/*
void RE_CompParticleEmitter::Update()
{
	int spawns_needed = 0;
	float dt = 0.f;// RE_TIME->GetDeltaTime();

	if (emissor_life > 0.f)
	{
		time_counter += dt;
		if (time_counter <= emissor_life)
		{
			spawn_counter += dt;
			while (spawn_counter > 1000.f / emissionRate)
			{
				spawn_counter -= (1000.f / emissionRate);
				spawns_needed++;
			}
		}
	}
	else
	{
		spawn_counter += dt;
		while (spawn_counter > 1.f / emissionRate)
		{
			spawn_counter -= (1.f / emissionRate);
			spawns_needed++;
		}
	}

	UpdateParticles(spawns_needed);
}

void RE_CompParticleEmitter::PostUpdate() {}

void RE_CompParticleEmitter::OnPlay()
{
	//RE_LOG_SECONDARY("particle emitter play");

	if (particles)
		DEL_A(particles);

	particles = new Particle[max_particles = (unsigned int)(emissionRate * (lifetime + lifetime_margin))];

	time_counter = 0.0f;
	spawn_counter = 0.0f;

	for (int i = 0; i < max_particles; i++)
		particles[i].SetUp(this);
}

void RE_CompParticleEmitter::OnPause() {}

void RE_CompParticleEmitter::OnStop()
{
	//RE_LOG_SECONDARY("particle emitter stop");
}
*/


/*
bool RE_CompParticleEmitter::LocalEmission() const { return local_emission; }
bool RE_CompParticleEmitter::EmmissionFinished() const { return emissor_life < 0.f ? false : time_counter <= emissor_life; }
RE_Mesh * RE_CompParticleEmitter::GetMesh() const { return mParticle; }


void RE_CompParticleEmitter::ResetParticle(Particle * p) { p->Emit(); }

void RE_CompParticleEmitter::UpdateParticles(int spawns_needed)
{
	Particle* p = nullptr;
	for (int i = 0; i < max_particles; i++)
	{
		if (!((p = &particles[i])->Alive()))
		{
			if (spawns_needed > 0)
			{
				ResetParticle(p);
				spawns_needed--;
			}
		}
		else p->Update();
	}
}
*/