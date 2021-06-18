#include "RE_CompParticleEmitter.h"

#include "RE_Profiler.h"
#include "Application.h"
#include "RE_Time.h"
#include "ModuleInput.h"
#include "ModulePhysics.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "RE_PrimitiveManager.h"
#include "ModuleRenderer3D.h"
#include "RE_CameraManager.h"
#include "RE_ShaderImporter.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_Mesh.h"
#include "RE_Material.h"
#include "RE_Shader.h"
#include "RE_GLCache.h"
#include "RE_Particle.h"
#include "RE_Json.h"

#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_CompPrimitive.h"

#include "ImGui\imgui.h"


RE_CompParticleEmitter::RE_CompParticleEmitter() : RE_Component(C_PARTICLEEMITER) {
}

RE_CompParticleEmitter::~RE_CompParticleEmitter()
{
	if (simulation != nullptr)
		RE_PHYSICS->RemoveEmitter(simulation);
}

RE_ParticleEmitter* RE_CompParticleEmitter::AddSimulation()
{
	if (simulation == nullptr)
		simulation = RE_PHYSICS->AddEmitter();

	return simulation;
}

void RE_CompParticleEmitter::CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent)
{}

void RE_CompParticleEmitter::Update()
{
	if (simulation)
	{
		RE_GameObject* g_obj = GetGOPtr();
		const math::vec global_pos = g_obj->GetTransformPtr()->GetGlobalPosition();
		simulation->parent_speed = (global_pos - simulation->parent_pos) / RE_TIME->GetDeltaTime();
		simulation->parent_pos = global_pos;

		if (simulation->state <= RE_ParticleEmitter::PlaybackState::PLAY)
			RE_INPUT->Push(TRANSFORM_MODIFIED, RE_SCENE, go);
	}
}

void RE_CompParticleEmitter::Draw() const
{
	RE_PROFILE(PROF_DrawParticles, PROF_CompParticleEmitter);

	if (!simulation->active_rendering) return;

#ifdef PARTICLE_RENDER_TEST

	RE_Timer timer_simple;

#endif // PARTICLE_RENDER_TEST

	// Get Shader and uniforms
	const RE_Shader* pS = static_cast<RE_Shader*>(RE_RES->At(RE_RES->internalResources->GetParticleShader()));
	const unsigned int shader = pS->GetID();
	RE_GLCache::ChangeShader(shader);

	RE_ShaderImporter::setFloat(shader, "useColor", 1.0f);
	RE_ShaderImporter::setFloat(shader, "useTexture", 0.0f);

	// Get GO Transform
	RE_CompTransform* transform = static_cast<RE_CompTransform*>(pool_gos->AtCPtr(go)->GetCompPtr(C_TRANSFORM));
	const math::float3 goPosition = transform->GetGlobalPosition();
	const math::float3 goUp = transform->GetUp().Normalized();

	// Get Camera Transform
	RE_CompTransform* cT = ModuleRenderer3D::GetCamera()->GetTransform();
	const math::float3 cUp = cT->GetUp().Normalized();

	for (auto p : simulation->particle_pool)
	{
		// Calculate Particle Transform
		const math::float3 partcleGlobalpos = simulation->local_space ? goPosition + p->position : p->position;
		math::float3 front, right, up;
		switch (simulation->particleDir)
		{
		case RE_ParticleEmitter::PS_FromPS:
		{
			front = (goPosition - partcleGlobalpos).Normalized();
			right = front.Cross(goUp).Normalized();
			if (right.x < 0.0f) right *= -1.0f;
			up = right.Cross(front).Normalized();
			break; 
		}
		case RE_ParticleEmitter::PS_Billboard:
		{
			front = cT->GetGlobalPosition() - partcleGlobalpos;
			front.Normalize();
			right = front.Cross(cUp).Normalized();
			if (right.x < 0.0f) right *= -1.0f;
			up = right.Cross(front).Normalized();
			break; 
		}
		case RE_ParticleEmitter::PS_Custom:
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
			if(cNormal) RE_ShaderImporter::setFloat(shader, "normal", front);
			RE_ShaderImporter::setFloat(shader, "specular", 2.5f);
			RE_ShaderImporter::setFloat(shader, "shininess", 16.0f);
			RE_ShaderImporter::setFloat(shader, "opacity", 1.0f);
		}
		else
		{
			//Opacity
			switch (simulation->opacity.type) {
			case RE_PR_Opacity::Type::OVERLIFETIME: weight = p->lifetime / simulation->initial_lifetime.GetMax(); break;
			case RE_PR_Opacity::Type::OVERDISTANCE: weight = math::SqrtFast(p->position.LengthSq()) / math::SqrtFast(simulation->max_dist_sq); break;
			case RE_PR_Opacity::Type::OVERSPEED: weight = math::SqrtFast(p->velocity.LengthSq()) / math::SqrtFast(simulation->max_speed_sq); break;
			default: break; }

			RE_ShaderImporter::setFloat(shader, "opacity", simulation->opacity.GetValue(weight));
		}

		// Color
		switch (simulation->color.type) {
		case RE_PR_Color::Type::OVERLIFETIME: weight = p->lifetime / simulation->initial_lifetime.GetMax(); break;
		case RE_PR_Color::Type::OVERDISTANCE: weight = math::SqrtFast(p->position.LengthSq()) / math::SqrtFast(simulation->max_dist_sq);break;
		case RE_PR_Color::Type::OVERSPEED: weight = math::SqrtFast(p->velocity.LengthSq()) / math::SqrtFast(simulation->max_speed_sq); break;
		default: break; }

		RE_ShaderImporter::setFloat(shader, "cdiffuse", simulation->color.GetValue(weight));

		// Draw Call
		if (simulation->meshMD5) dynamic_cast<RE_Mesh*>(RE_RES->At(simulation->meshMD5))->DrawMesh(shader);
		else dynamic_cast<RE_CompPrimitive*>(simulation->primCmp)->SimpleDraw();
	}

#ifdef PARTICLE_RENDER_TEST

	ProfilingTimer::update_time = RE_Math::MaxUI(timer_simple.Read(), ProfilingTimer::update_time);
	if (ProfilingTimer::update_time > 33u)
	{
		RE_PHYSICS->mode = ModulePhysics::UpdateMode::ENGINE_PAR;

		eastl::string file_name = "Particle_Rendering ";
		file_name += eastl::to_string(ProfilingTimer::current_sim / 10);
		file_name += eastl::to_string(ProfilingTimer::current_sim % 10);

		// TODO JULIUS: setup filename from emitter properties for rendering profiling

		file_name += ".json";

		RE_Profiler::Deploy(file_name.c_str());
		ProfilingTimer::current_sim < 11 // MAX SIMULATIONS INDEX
			? RE_ParticleEmitter::demo_emitter->DemoSetup() : App->QuickQuit();
	}
#endif // PARTICLE_RENDER_TEST
}

void RE_CompParticleEmitter::DrawProperties()
{
	if (ImGui::CollapsingHeader("Particle Emitter"))
	{
		if (simulation != nullptr)
		{
			if (ImGui::Button("Edit simulation on workspace"))
				RE_EDITOR->StartEditingParticleEmiter(simulation, id);
		}
		else
		{
			ImGui::Text("Unregistered simulation");
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
	if (emitter_md5) RE_RES->Use(emitter_md5);
}

void RE_CompParticleEmitter::UnUseResources()
{
	if (emitter_md5) RE_RES->UnUse(emitter_md5);
}

bool RE_CompParticleEmitter::isLighting() const { return simulation->light.type; }

void RE_CompParticleEmitter::CallLightShaderUniforms(unsigned int shader, const char* u_name, unsigned int& count, unsigned int maxLights, bool sharedLight) const
{
	eastl::list<RE_Particle*>* particles = RE_PHYSICS->GetParticles(simulation->id);
	RE_CompTransform* transform = GetGOPtr()->GetTransformPtr();
	const math::vec objectPos = transform->GetGlobalPosition();

	eastl::string array_name(u_name);
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
		
		for (auto p : *particles)
		{
			if (count == maxLights) return;
			unif_name = array_name + eastl::to_string(count++) + "].";
			const math::float3 p_global_pos = objectPos + p->position;

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
					p->lightColor.x, p->lightColor.y, p->lightColor.z, p->specular);

				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "positionIntensity").c_str()),
					p_global_pos.x, p_global_pos.y, p_global_pos.z, p->intensity);

				break;
			}
			default: break; }
		}
	}
	else
	{
		const math::vec color = simulation->light.GetColor();
		const float intensity = simulation->light.GetIntensity();

		for (auto p : *particles)
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
					0.f, 0.f, 0.f, p->intensity);

				RE_ShaderImporter::setFloat(
					RE_ShaderImporter::getLocation(shader, (unif_name + "diffuseSpecular").c_str()),
					p->lightColor.x, p->lightColor.y, p->lightColor.z, p->specular);
				break;
			}
			default: break; }

			const math::float3 partcleGlobalpos = objectPos + p->position;
			RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "positionType").c_str()), partcleGlobalpos.x, partcleGlobalpos.y, partcleGlobalpos.z, static_cast<float>(L_POINT));
			
			const math::vec quadratic = simulation->light.GetQuadraticValues();
			RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(shader, (unif_name + "clq").c_str()), quadratic.x, quadratic.y, quadratic.z, 0.f);
		}
	}
}

bool RE_CompParticleEmitter::isBlend() const { return static_cast<bool>(simulation->opacity.type); }

RE_ParticleEmitter* RE_CompParticleEmitter::GetSimulation() const
{
	return simulation;
}