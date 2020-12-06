#include "RE_Prefab.h"

#include "Application.h"
#include "ModuleScene.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_ECS_Manager.h"
#include "RE_LogManager.h"
#include "md5.h"
#include "RE_ECS_Importer.h"
#include "Globals.h"

#include "ImGui/imgui.h"
#include <EASTL/map.h>

RE_Prefab::RE_Prefab() {}
RE_Prefab::RE_Prefab(const char* metaPath) : ResourceContainer(metaPath) {}
RE_Prefab::~RE_Prefab() {}

void RE_Prefab::LoadInMemory()
{
	if (App::fs->Exists(GetLibraryPath()))
	{
		LibraryLoad();
	}
	else if (App::fs->Exists(GetAssetPath()))
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

void RE_Prefab::Save(RE_ECS_Manager* pool, bool rootidentity, bool keepInMemory)
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

RE_ECS_Manager* RE_Prefab::GetPool()
{
	RE_ECS_Manager* ret;
	bool unload = false;
	if (unload = (loaded == nullptr)) LoadInMemory();
	ret = loaded->GetNewPoolFromID(loaded->GetRootUID());
	if (unload) UnloadMemory();
	return ret;
}

void RE_Prefab::AssetSave()
{
	//Serialize
	Config prefab_SaveFile(GetAssetPath(), App::fs->GetZipPath());
	JSONNode* prefabNode = prefab_SaveFile.GetRootNode("prefab");

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
	Config jsonLoad(GetAssetPath(), App::fs->GetZipPath());

	if (jsonLoad.Load())
	{
		JSONNode* prefabNode = jsonLoad.GetRootNode("prefab");
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
	RE_FileIO binaryLoad(GetLibraryPath());
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
	RE_FileIO toLibrarySave(GetLibraryPath(), App::fs->GetZipPath());
	toLibrarySave.Save(buffer, size);
	DEL_A(buffer);
}

void RE_Prefab::Draw()
{
	if (ImGui::Button("Add to Scene")) {
		if (loaded == nullptr) App::resources->Use(GetMD5());
		App::scene->AddGOPool(loaded);
	}
}
