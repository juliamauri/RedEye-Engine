#include "RE_ParticleEmitterBase.h"

#include "Globals.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"
#include "RE_ResourceManager.h"

void RE_ParticleEmitterBase::LoadInMemory()
{
	if (RE_FS->Exists(GetLibraryPath()))
	{
		LibraryLoad();
	}
	else if (RE_FS->Exists(GetAssetPath()))
	{
		AssetLoad();
		LibrarySave();
	}
	else RE_LOG_ERROR("Emitter Base %s not found in project", GetName());
}

void RE_ParticleEmitterBase::UnloadMemory()
{
	resource_emission = nullptr;
	resource_renderer = nullptr;

	ResourceContainer::inMemory = false;
}

void RE_ParticleEmitterBase::Import(bool keepInMemory)
{
	AssetLoad(true);
	LibrarySave();
	if (!keepInMemory) UnloadMemory();
}

void RE_ParticleEmitterBase::SomeResourceChanged(const char* resMD5)
{
	if (resource_emission == resMD5)
	{
		if (!isInMemory())
		{
			LoadInMemory();
			ResourceContainer::inMemory = false;
		}

		Save();
	}
	else if (resource_renderer == resMD5)
	{
		if (!isInMemory())
		{
			LoadInMemory();
			ResourceContainer::inMemory = false;
		}

		Save();
	}
}

void RE_ParticleEmitterBase::Save()
{
	JsonSerialize();
	BinarySerialize();
	SaveMeta();
}

void RE_ParticleEmitterBase::ProcessMD5()
{
	//md5 from emissor md5 + render md5
	SetMD5((eastl::string(resource_emission) + resource_renderer).c_str());
}


void RE_ParticleEmitterBase::Draw()
{
	if (!isInternal() && applySave && ImGui::Button("Save Changes"))
	{
		Save();
		RE_RENDER->PushThumnailRend(GetMD5(), true);
		applySave = false;
	}

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

void RE_ParticleEmitterBase::JsonDeserialize(bool generateLibraryPath)
{
	Config particles(GetAssetPath(), RE_FS->GetZipPath());
	if (particles.Load())
	{
		RE_Json* node = particles.GetRootNode("Particles");


		if (generateLibraryPath)
		{
			SetMD5(particles.GetMd5().c_str());
			eastl::string libraryPath("Library/Particles/");
			libraryPath += GetMD5();
			SetLibraryPath(libraryPath.c_str());
		}

		ResourceContainer::inMemory = true;
	}
}

void RE_ParticleEmitterBase::JsonSerialize(bool onlyMD5)
{
	Config emission(GetAssetPath(), RE_FS->GetZipPath());
	RE_Json* node = emission.GetRootNode("Particles");



	//We need to call ProcessMD5() before SaveMeta



	if (!onlyMD5) emission.Save();
	SetMD5(emission.GetMd5().c_str());

	eastl::string libraryPath("Library/Particles/");
	libraryPath += GetMD5();
	SetLibraryPath(libraryPath.c_str());

	DEL(node);
}




void RE_ParticleEmitterBase::AssetLoad(bool generateLibraryPath)
{
	Config jsonLoad(GetAssetPath(), RE_FS->GetZipPath());

	if (jsonLoad.Load())
	{
		RE_Json* node = jsonLoad.GetRootNode("Particles");
		JsonDeserialize(prefabNode);
		DEL(node);

		if (generateLibraryPath)
		{
			eastl::string md5 = jsonLoad.GetMd5();
			SetMD5(md5.c_str());
			eastl::string libraryPath("Library/Particles/");
			libraryPath += md5;
			SetLibraryPath(libraryPath.c_str());
		}
	}

	ResourceContainer::inMemory = true;
}

void RE_ParticleEmitterBase::LibraryLoad()
{
	RE_FileBuffer binaryLoad(GetLibraryPath());
	if (binaryLoad.Load())
	{
		char* cursor = binaryLoad.GetBuffer();
		loaded = RE_ECS_Importer::BinaryDeserialize(cursor);
	}
	ResourceContainer::inMemory = true;
}

void RE_ParticleEmitterBase::LibrarySave()
{
	RE_FileBuffer assetFile(GetAssetPath());
	RE_FileBuffer libraryFile(GetLibraryPath(), RE_FS->GetZipPath());
	if (assetFile.Load()) RE_TextureImporter::SaveOwnFormat(assetFile.GetBuffer(), assetFile.GetSize(), texType, &libraryFile);
}

void RE_ParticleEmitterBase::BinaryDeserialize()
{
}

void RE_ParticleEmitterBase::BinarySerialize()
{
}

unsigned int RE_ParticleEmitterBase::GetBinarySize() const
{
	return 0;
}

