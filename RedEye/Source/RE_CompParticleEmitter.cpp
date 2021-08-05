#include "RE_CompParticleEmitter.h"

#include "RE_Profiler.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModulePhysics.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "RE_ResourceManager.h"

#include "RE_ParticleEmitterBase.h"
#include "RE_ShaderImporter.h"

#include "RE_Time.h"
#include "RE_Json.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"

#include "ImGui\imgui.h"

RE_CompParticleEmitter::RE_CompParticleEmitter() : RE_Component(C_PARTICLEEMITER) {}

RE_CompParticleEmitter::~RE_CompParticleEmitter() {}


void RE_CompParticleEmitter::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent)
{
	pool_gos = pool;
	if (go = parent) pool_gos->AtPtr(go)->ReportComponent(id, C_PARTICLEEMITER);

	RE_CompParticleEmitter* cmp_emitter = dynamic_cast<RE_CompParticleEmitter*>(copy);
	simulation = cmp_emitter->simulation;
	emitter_md5 = cmp_emitter->emitter_md5;
}

void RE_CompParticleEmitter::Update()
{
	if (simulation)
	{
		RE_GameObject* g_obj = GetGOPtr();
		const math::vec global_pos = g_obj->GetTransformPtr()->GetGlobalPosition();
		simulation->parent_speed = (global_pos - simulation->parent_pos) / RE_TIME->GetDeltaTime();
		simulation->parent_pos = global_pos;

		if (simulation->state <= RE_ParticleEmitter::PlaybackState::PLAY)
			RE_INPUT->Push(RE_EventType::TRANSFORM_MODIFIED, RE_SCENE, go);
	}
}

void RE_CompParticleEmitter::Draw() const
{
	RE_CompTransform* transform = static_cast<RE_CompTransform*>(pool_gos->AtCPtr(go)->GetCompPtr(C_TRANSFORM));
	RE_PHYSICS->DrawParticleEmitterSimulation(simulation->id, transform->GetGlobalPosition(), transform->GetUp().Normalized());
}

void RE_CompParticleEmitter::DrawProperties()
{
	if (ImGui::CollapsingHeader("Particle Emitter"))
	{
		if (emitter_md5)
		{
			if (ImGui::Button("Go to emitter resource"))
				RE_RES->PushSelected(emitter_md5, true);
		}
		else
			ImGui::Text("There is not particle emitter resource selected, using basic emitter.");
		
		if (ImGui::BeginMenu("Change particle emitter"))
		{
			eastl::vector<ResourceContainer*> r_emitters = RE_RES->GetResourcesByType(Resource_Type::R_PARTICLE_EMITTER);
			bool none = true;
			for (auto emitter : r_emitters)
			{
				if (emitter->isInternal()) continue;

				none = false;
				if (ImGui::MenuItem(emitter->GetName()))
				{
					UnUseResources();
					emitter_md5 = emitter->GetMD5();
					UseResources();
				}
			}
			if (none) ImGui::Text("No custom emitters on assets");
			ImGui::EndMenu();
		}
	}
}

void RE_CompParticleEmitter::SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->PushInt("emitterResource", (emitter_md5) ? resources->at(emitter_md5) : -1);
}

void RE_CompParticleEmitter::DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources)
{
	int id = node->PullInt("emitterResource", -1);
	emitter_md5 = (id != -1) ? resources->at(id) : nullptr;
}

unsigned int RE_CompParticleEmitter::GetBinarySize() const { return sizeof(int); }

void RE_CompParticleEmitter::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(int);
	int md5 = (emitter_md5) ? resources->at(emitter_md5) : -1;
	memcpy(cursor, &md5, size);
	cursor += size;
}

void RE_CompParticleEmitter::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(int);
	int md5 = -1;
	memcpy(&md5, cursor, size);
	cursor += size;

	emitter_md5 = (md5 != -1) ? resources->at(md5) : nullptr;
}

eastl::vector<const char*> RE_CompParticleEmitter::GetAllResources()
{
	eastl::vector<const char*> ret;
	if (emitter_md5) ret.push_back(emitter_md5);
	return ret;
}

void RE_CompParticleEmitter::UseResources()
{
	if (emitter_md5) {
		RE_RES->Use(emitter_md5);
		simulation = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emitter_md5))->GetNewEmitter();
	}
	else
		simulation = new RE_ParticleEmitter(true);

	RE_PHYSICS->AddEmitter(simulation);
}

void RE_CompParticleEmitter::UnUseResources()
{
	if (emitter_md5) RE_RES->UnUse(emitter_md5);
	RE_PHYSICS->RemoveEmitter(simulation);
}

bool RE_CompParticleEmitter::isLighting() const { return simulation->light.type; }

void RE_CompParticleEmitter::CallLightShaderUniforms(unsigned int shader, const char* array_unif_name, unsigned int& count, unsigned int maxLights, bool sharedLight) const
{
	RE_CompTransform* transform = static_cast<RE_CompTransform*>(pool_gos->AtCPtr(go)->GetCompPtr(C_TRANSFORM));
	RE_PHYSICS->CallParticleEmitterLightShaderUniforms(simulation->id, transform->GetGlobalPosition(), shader, array_unif_name, count, maxLights, sharedLight);
}

bool RE_CompParticleEmitter::isBlend() const { return static_cast<bool>(simulation->opacity.type); }

RE_ParticleEmitter* RE_CompParticleEmitter::GetSimulation() const
{
	return simulation;
}

const char* RE_CompParticleEmitter::GetEmitterResource() const
{
	return emitter_md5;
}

void RE_CompParticleEmitter::UpdateEmitter(const char* emitter)
{
	if (emitter == emitter_md5)
		dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emitter_md5))->FillEmitter(simulation);
}

void RE_CompParticleEmitter::SetEmitter(const char* md5)
{
	emitter_md5 = md5;
}
