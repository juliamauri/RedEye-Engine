#include "Resource.h"

#include "RE_Prefab.h"

#include "RE_Memory.h"
#include "RE_ConsoleLog.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"
#include "Application.h"
#include "ModuleScene.h"
#include "RE_ResourceManager.h"
#include "RE_ECS_Pool.h"
#include "RE_ECS_Importer.h"

#include <ImGui/imgui.h>
#include <EASTL/map.h>

RE_Prefab::RE_Prefab() {}
RE_Prefab::RE_Prefab(const char* metaPath) : ResourceContainer(metaPath) {}
RE_Prefab::~RE_Prefab() {}

void RE_Prefab::LoadInMemory()
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
	else {
		RE_LOG_ERROR("Prefab %s not found in project", GetName());
	}
}

void RE_Prefab::UnloadMemory()
{
	DEL(loaded);
	DEL(toSave);
	ResourceContainer::inMemory = false;
}

void RE_Prefab::Import(bool keepInMemory)
{
	AssetLoad();
	LibrarySave();
	if (!keepInMemory) UnloadMemory();
}

void RE_Prefab::Save(RE_ECS_Pool* pool, bool rootidentity, bool keepInMemory)
{
	if (pool)
	{
		loaded = toSave = pool;
		RE_GameObject* root = loaded->GetGOPtr(0);
		if (rootidentity)
		{
			RE_CompTransform* t = root->GetTransformPtr();
			t->SetPosition(math::vec::zero);
			t->SetRotation(math::vec::zero);
			t->SetScale(math::vec::one);
			root->ResetGOandChildsAABB();
		}
		AssetSave();
		LibrarySave();
		if (!keepInMemory) DEL(loaded);
		ResourceContainer::inMemory = keepInMemory;
	}
}

void RE_Prefab::SetName(const char* _name)
{
	ResourceContainer::SetName(_name);
	eastl::string assetPath("Assets/Prefabs/");
	(assetPath += _name) += ".refab";
	SetAssetPath(assetPath.c_str());
}

RE_ECS_Pool* RE_Prefab::GetPool()
{
	RE_ECS_Pool* ret;
	bool unload = false;
	if (unload = (loaded == nullptr)) LoadInMemory();
	ret = loaded->GetNewPoolFromID(loaded->GetRootUID());
	if (unload) UnloadMemory();
	return ret;
}

void RE_Prefab::AssetSave()
{
	//Serialize
	Config prefab_SaveFile(GetAssetPath());
	RE_Json* prefabNode = prefab_SaveFile.GetRootNode("prefab");

	RE_ECS_Importer::JsonSerialize(prefabNode, toSave);
	DEL(prefabNode);

	//Setting LibraryPath and MD5
	eastl::string md5 = prefab_SaveFile.GetMd5();
	SetMD5(md5.c_str());
	eastl::string libraryPath("Library/Prefabs/");
	libraryPath += md5;
	SetLibraryPath(libraryPath.c_str());

	//Save
	prefab_SaveFile.Save();
}

void RE_Prefab::AssetLoad(bool generateLibraryPath)
{
	Config jsonLoad(GetAssetPath());

	if (jsonLoad.Load())
	{
		RE_Json* prefabNode = jsonLoad.GetRootNode("prefab");
		loaded = RE_ECS_Importer::JsonDeserialize(prefabNode);
		DEL(prefabNode);

		if (generateLibraryPath)
		{
			eastl::string md5 = jsonLoad.GetMd5();
			SetMD5(md5.c_str());
			eastl::string libraryPath("Library/Prefabs/");
			libraryPath += md5;
			SetLibraryPath(libraryPath.c_str());
		}
	}

	ResourceContainer::inMemory = true;
}

void RE_Prefab::LibraryLoad()
{
	RE_FileBuffer binaryLoad(GetLibraryPath());
	if (binaryLoad.Load())
	{
		char* cursor = binaryLoad.GetBuffer();
		loaded = RE_ECS_Importer::BinaryDeserialize(cursor);
	}
	ResourceContainer::inMemory = true;
}

void RE_Prefab::LibrarySave()
{
	uint size = 0;
	char* buffer = RE_ECS_Importer::BinarySerialize(toSave, &size);
	RE_FileBuffer toLibrarySave(GetLibraryPath());
	toLibrarySave.Save(buffer, size);
	DEL_A(buffer);
}

void RE_Prefab::Draw()
{
	if (ImGui::Button("Add to Scene")) {
		if (loaded == nullptr) RE_RES->Use(GetMD5());
		RE_SCENE->AddGOPool(loaded);
	}
}
