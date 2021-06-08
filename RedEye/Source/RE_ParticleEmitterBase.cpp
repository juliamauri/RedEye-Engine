#include "RE_ParticleEmitterBase.h"

#include "Globals.h"
#include "Application.h"
#include "RE_Json.h"
#include "RE_ResourceManager.h"

#include"md5.h"

void RE_ParticleEmitterBase::LoadInMemory()
{
	RE_RES->At(resource_emission)->LoadInMemory();
	RE_RES->At(resource_renderer)->LoadInMemory();
	ResourceContainer::inMemory = true;
}

void RE_ParticleEmitterBase::UnloadMemory()
{
	RE_RES->At(resource_emission)->UnloadMemory();
	RE_RES->At(resource_renderer)->UnloadMemory();
	ResourceContainer::inMemory = false;
}

void RE_ParticleEmitterBase::SomeResourceChanged(const char* resMD5)
{
	MD5 domd5(eastl::string(resource_emission) + eastl::string(resource_renderer));
	SetMD5(domd5.hexdigest().c_str());

	if (resource_emission == resMD5)
	{
		if (isInMemory()) {
			//Do changes to scene
		}
		SaveMeta();
	}
	else if (resource_renderer == resMD5)
	{
		if (isInMemory()) {
			//Do changes to scene
		}
		SaveMeta();
	}
}

void RE_ParticleEmitterBase::Draw()
{
	//if (!isInternal() && applySave && ImGui::Button("Save Changes"))
	//{
	//	Save();
	//	RE_RENDER->PushThumnailRend(GetMD5(), true);
	//	applySave = false;
	//}

	//MD5 domd5(eastl::string(resource_emission) + eastl::string(resource_renderer));
	//SetMD5(domd5.hexdigest().c_str());


	//Button workspace particle edittor

	//Drag and drop resources emission and render


	//ImGui::Image(reinterpret_cast<void*>(RE_EDITOR->thumbnails->At(GetMD5())), { 256, 256 }, { 0,1 }, { 1, 0 });
}

void RE_ParticleEmitterBase::SaveResourceMeta(RE_Json* metaNode)
{
	metaNode->PushString("Emission Meta", (resource_emission) ? RE_RES->At(resource_emission)->GetMetaPath() : "NOMETAPATH");
	metaNode->PushString("Rendering Meta", (resource_renderer) ? RE_RES->At(resource_renderer)->GetMetaPath() : "NOMETAPATH");
}

void RE_ParticleEmitterBase::LoadResourceMeta(RE_Json* metaNode)
{
	eastl::string tmp = metaNode->PullString("Emission Meta", "NOMETAPATH");
	if (tmp.compare("NOMETAPATH") != 0) resource_emission = RE_RES->FindMD5ByMETAPath(tmp.c_str(), Resource_Type::R_PARTICLE_EMISSION);
	
	tmp = metaNode->PullString("Rendering Meta", "NOMETAPATH");
	if (tmp.compare("NOMETAPATH") != 0) resource_renderer = RE_RES->FindMD5ByMETAPath(tmp.c_str(), Resource_Type::R_PARTICLE_RENDER);
}