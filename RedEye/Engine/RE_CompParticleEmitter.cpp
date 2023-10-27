#include "RE_CompParticleEmitter.h"

#include "RE_Time.h"
#include "RE_Profiler.h"
#include "RE_Json.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"

#include "RE_ParticleManager.h"
#include "RE_ResourceManager.h"
#include "RE_CameraManager.h"

#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_ParticleEmitterBase.h"

#include <MGL/Math/float3.h>
#include <ImGui/imgui.h>

RE_CompParticleEmitter::RE_CompParticleEmitter() :
	RE_Component(RE_Component::Type::PARTICLEEMITER),
	simulation(RE_ParticleManager::Allocate(*(new RE_ParticleEmitter())))
{}

void RE_CompParticleEmitter::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent)
{
	pool_gos = pool;
	if (go = parent) pool_gos->AtPtr(go)->ReportComponent(id, RE_Component::Type::PARTICLEEMITER);

	auto cmp_emitter = dynamic_cast<RE_CompParticleEmitter*>(copy);
	simulation = cmp_emitter->simulation;
	emitter_md5 = cmp_emitter->emitter_md5;
}

void RE_CompParticleEmitter::Update()
{
	if (!simulation) return;

	RE_GameObject* g_obj = GetGOPtr();
	auto global_pos = g_obj->GetTransformPtr()->GetGlobalPosition();
	if (RE_ParticleManager::UpdateEmitter(simulation, global_pos))
		RE_INPUT->Push(RE_EventType::TRANSFORM_MODIFIED, RE_SCENE, go);
}

void RE_CompParticleEmitter::Draw() const
{
	auto transform = dynamic_cast<RE_CompTransform*>(pool_gos->AtCPtr(go)->GetCompPtr(RE_Component::Type::TRANSFORM));
	RE_ParticleManager::DrawSimulation(
		simulation,
		RE_CameraManager::MainCamera()->Camera,
		transform->GetGlobalPosition(),
		transform->GetUp().Normalized());
}

void RE_CompParticleEmitter::DrawProperties()
{
	if (!ImGui::CollapsingHeader("Particle Emitter"))
		return;

	if (!emitter_md5)
		ImGui::Text("There is not particle emitter resource selected, using basic emitter.");
	else if (ImGui::Button("Go to emitter resource"))
		RE_RES->PushSelected(emitter_md5, true);
		

	if (!ImGui::BeginMenu("Change particle emitter"))
		return;

	bool none = true;
	for (auto emitter : RE_RES->GetResourcesByType(ResourceContainer::Type::PARTICLE_EMITTER))
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

bool RE_CompParticleEmitter::HasBlend() const { return RE_ParticleManager::EmitterHasBlend(simulation); }
bool RE_CompParticleEmitter::HasLight() const { return RE_ParticleManager::EmitterHasLight(simulation); }

void RE_CompParticleEmitter::UpdateEmitter(const char* emitter)
{
	if (emitter != emitter_md5) return;

	RE_ParticleEmitter* to_fill = RE_ParticleManager::GetEmitter(simulation);
	if (!to_fill) return;
	
	dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emitter_md5))->FillEmitter(to_fill);
}

#pragma region Resources

void RE_CompParticleEmitter::UseResources()
{
	RE_ParticleEmitter* emitter = nullptr;
	if (emitter_md5)
	{
		RE_RES->Use(emitter_md5);
		emitter = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(emitter_md5))->GetNewEmitter();
	}
	else emitter = new RE_ParticleEmitter(true);

	simulation = RE_ParticleManager::Allocate(*emitter);
}

void RE_CompParticleEmitter::UnUseResources()
{
	if (emitter_md5) RE_RES->UnUse(emitter_md5);
	RE_ParticleManager::Deallocate(simulation);
}

eastl::vector<const char*> RE_CompParticleEmitter::GetAllResources()
{
	eastl::vector<const char*> ret;
	if (emitter_md5) ret.push_back(emitter_md5);
	return ret;
}

#pragma endregion

#pragma region Serialization

void RE_CompParticleEmitter::JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const
{
	node->Push("emitterResource", (emitter_md5) ? resources->at(emitter_md5) : -1);
}

void RE_CompParticleEmitter::JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources)
{
	int id = node->PullInt("emitterResource", -1);
	emitter_md5 = (id != -1) ? resources->at(id) : nullptr;
}

size_t RE_CompParticleEmitter::GetBinarySize() const { return sizeof(int); }

void RE_CompParticleEmitter::BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const
{
	size_t size = sizeof(int);
	int md5 = (emitter_md5) ? resources->at(emitter_md5) : -1;
	memcpy(cursor, &md5, size);
	cursor += size;
}

void RE_CompParticleEmitter::BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources)
{
	size_t size = sizeof(int);
	int md5 = -1;
	memcpy(&md5, cursor, size);
	cursor += size;

	emitter_md5 = (md5 != -1) ? resources->at(md5) : nullptr;
}

#pragma endregion