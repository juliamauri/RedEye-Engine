#include "Resource.h"

#include "RE_ParticleEmitterBase.h"

#include "RE_Memory.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "RE_Json.h"
#include "RE_PrimitiveManager.h"
#include "RE_ResourceManager.h"
#include "RE_ParticleEmission.h"
#include "RE_ParticleRender.h"
#include "RE_ParticleEmitter.h"
#include "RE_CompParticleEmitter.h"
#include "ParticleEmitterEditorWindow.h"

#include <MD5/md5.h>

void RE_ParticleEmitterBase::LoadInMemory()
{
	if(resource_emission) RE_RES->At(resource_emission)->LoadInMemory();
	if (resource_renderer) RE_RES->At(resource_renderer)->LoadInMemory();
	ResourceContainer::inMemory = true;
}

void RE_ParticleEmitterBase::UnloadMemory()
{
	if (resource_emission) RE_RES->At(resource_emission)->UnloadMemory();
	if (resource_renderer) RE_RES->At(resource_renderer)->UnloadMemory();
	ResourceContainer::inMemory = false;
}

void RE_ParticleEmitterBase::SomeResourceChanged(const char* resMD5)
{
	if (resource_emission != resMD5 && resource_renderer != resMD5) return;

	eastl::string new_md5 = "";
	if (resource_emission) new_md5 = resource_emission;
	if (resource_renderer) new_md5 += resource_renderer;

	MD5 domd5(eastl::string(resource_emission) + eastl::string(resource_renderer));
	SetMD5(domd5.hexdigest().c_str());

	bool check_scene = false;
	if (resource_emission == resMD5)
	{
		if (isInMemory())
			check_scene = true;
		SaveMeta();
	}
	else if (resource_renderer == resMD5)
	{
		if (isInMemory())
			check_scene = true;
		SaveMeta();
	}

	if (check_scene) {
		eastl::vector<RE_Component*> emitters = RE_SCENE->GetScenePool()->GetAllCompPtr(RE_Component::Type::PARTICLEEMITER);
		for (auto c_emitter : emitters)
			dynamic_cast<RE_CompParticleEmitter*>(c_emitter)->UpdateEmitter(GetMD5());
	}
}

RE_ParticleEmitter* RE_ParticleEmitterBase::GetNewEmitter()
{
	RE_ParticleEmitter* ret = new RE_ParticleEmitter((!resource_renderer));

	if (resource_emission)
		dynamic_cast<RE_ParticleEmission*>(RE_RES->At(resource_emission))->FillEmitter(ret);

	if (resource_renderer)
		dynamic_cast<RE_ParticleRender*>(RE_RES->At(resource_renderer))->FillEmitter(ret);

	return ret;
}

void RE_ParticleEmitterBase::FillEmitter(RE_ParticleEmitter* emitter_to_fill)
{
	if (resource_emission)
		dynamic_cast<RE_ParticleEmission*>(RE_RES->At(resource_emission))->FillEmitter(emitter_to_fill);

	if (resource_renderer)
		dynamic_cast<RE_ParticleRender*>(RE_RES->At(resource_renderer))->FillEmitter(emitter_to_fill);
	else
	{
		if (emitter_to_fill->primCmp)
		{
			emitter_to_fill->primCmp->UnUseResources();
			DEL(emitter_to_fill->primCmp)
			emitter_to_fill->primCmp = new RE_CompPoint();
			RE_SCENE->primitives->SetUpComponentPrimitive(emitter_to_fill->primCmp);

		}
	}
}

void RE_ParticleEmitterBase::FillAndSave(RE_ParticleEmitter* for_fill)
{
	eastl::string new_md5 = "";
	if (resource_emission) {
		RE_ParticleEmission* emission = dynamic_cast<RE_ParticleEmission*>(RE_RES->At(resource_emission));
		emission->FillResouce(for_fill);
		emission->Save();
		new_md5 = emission->GetMD5();
	}

	if (resource_renderer) {
		RE_ParticleRender* renderer = dynamic_cast<RE_ParticleRender*>(RE_RES->At(resource_renderer));
		renderer->FillResouce(for_fill);
		renderer->Save();
		new_md5 += renderer->GetMD5();
	}
	SetMD5(MD5(new_md5).hexdigest().c_str());
	SetMetaPath("Assets/Particles/");
	SaveMeta();
}

void RE_ParticleEmitterBase::GenerateSubResourcesAndReference(const char* emission_name, const char* renderer_name)
{
	if (!resource_emission) {
		RE_ParticleEmission* new_emission = new RE_ParticleEmission();
		new_emission->SetName(emission_name);
		new_emission->SetType(ResourceContainer::Type::PARTICLE_EMISSION);
		new_emission->SetAssetPath(eastl::string("Assets/Particles/"
			+ eastl::string(emission_name) + ".lasse").c_str());
		resource_emission = RE_RES->Reference(new_emission);
	}

	if (!resource_renderer) {
		RE_ParticleRender* new_renderer = new RE_ParticleRender();
		new_renderer->SetName(renderer_name);
		new_renderer->SetType(ResourceContainer::Type::PARTICLE_RENDER);
		new_renderer->SetAssetPath(eastl::string("Assets/Particles/"
			+ eastl::string(renderer_name) + ".lopfe").c_str());
		resource_renderer = RE_RES->Reference(new_renderer);
	}
}

void RE_ParticleEmitterBase::ChangeEmissor(RE_ParticleEmitter* sim, const char* emissor)
{
	if (inMemory && resource_emission) RE_RES->UnUse(resource_emission);
	resource_emission = emissor;
	if (resource_emission) {
		RE_RES->Use(resource_emission);
		if(sim) dynamic_cast<RE_ParticleEmission*>(RE_RES->At(resource_emission))->FillEmitter(sim);
	}
	else
	{
		RE_ParticleEmission tmp;
		if (sim) tmp.FillEmitter(sim);
	}
}

void RE_ParticleEmitterBase::ChangeRenderer(RE_ParticleEmitter* sim, const char* renderer)
{
	if (inMemory && resource_renderer) RE_RES->UnUse(resource_renderer);
	resource_renderer = renderer;
	if (resource_renderer) {
		RE_RES->Use(resource_renderer);
		if (sim) dynamic_cast<RE_ParticleRender*>(RE_RES->At(resource_renderer))->FillEmitter(sim);
	}
	else
	{
		RE_ParticleRender tmp;
		if (sim) tmp.FillEmitter(sim);
	}
}

bool RE_ParticleEmitterBase::HasEmissor() const
{
	return (resource_emission);
}

bool RE_ParticleEmitterBase::HasRenderer() const
{
	return (resource_renderer);
}

bool RE_ParticleEmitterBase::Contains(const char* res) const
{
	return (resource_emission == res || resource_renderer == res);
}

void RE_ParticleEmitterBase::Draw()
{
	static RE_ParticleEmitter* editting_emitter = nullptr;
	if (ImGui::Button("Edit particle emitter"))
	{
		RE_RES->Use(GetMD5());
		RE_EDITOR->GetParticleEmitterEditorWindow()->StartEditing(GetNewEmitter(), GetMD5());
	}

	//if (!isInternal() && applySave && ImGui::Button("Save Changes"))
	//{
	//	Save();
	//	RE_ThumbnailManager::AddThumbnail(GetMD5(), true);
	//	applySave = false;
	//}

	//MD5 domd5(eastl::string(resource_emission) + eastl::string(resource_renderer));
	//SetMD5(domd5.hexdigest().c_str());


	//Button workspace particle edittor

	//Drag and drop resources emission and render


	//ImGui::Image(reinterpret_cast<void*>(RE_EDITOR->thumbnails->At(GetMD5())), { 256, 256 }, { 0,1 }, { 1, 0 });
}

void RE_ParticleEmitterBase::SaveResourceMeta(RE_Json* metaNode) const
{
	metaNode->Push("Emission Meta", (resource_emission) ? RE_RES->At(resource_emission)->GetMetaPath() : "NOMETAPATH");
	metaNode->Push("Rendering Meta", (resource_renderer) ? RE_RES->At(resource_renderer)->GetMetaPath() : "NOMETAPATH");
}

void RE_ParticleEmitterBase::LoadResourceMeta(RE_Json* metaNode)
{
	eastl::string tmp = metaNode->PullString("Emission Meta", "NOMETAPATH");
	if (tmp.compare("NOMETAPATH") != 0) resource_emission = RE_RES->FindMD5ByMETAPath(tmp.c_str(), ResourceContainer::Type::PARTICLE_EMISSION);
	
	tmp = metaNode->PullString("Rendering Meta", "NOMETAPATH");
	if (tmp.compare("NOMETAPATH") != 0) resource_renderer = RE_RES->FindMD5ByMETAPath(tmp.c_str(), ResourceContainer::Type::PARTICLE_RENDER);
}