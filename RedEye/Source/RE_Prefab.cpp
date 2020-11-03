#include "RE_Prefab.h"

#include "Application.h"
#include "ModuleScene.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_GOManager.h"
#include  "OutputLog.h"
#include "md5.h"
#include "RE_ResouceAndGOImporter.h"
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
	if (loaded) DEL(loaded);
	if (toSave) DEL(toSave);
	loaded = nullptr;
	toSave = nullptr;
	ResourceContainer::inMemory = false;
}

void RE_Prefab::Import(bool keepInMemory)
{
	AssetLoad();
	LibrarySave();
	if (!keepInMemory) UnloadMemory();
}

void RE_Prefab::Save(RE_GOManager* pool, bool rootidentity, bool keepInMemory)
{
	if (pool)
	{
		loaded = toSave = pool;
		RE_GameObject* root = loaded->GetGO(0);
		if (rootidentity)
		{
			root->GetTransform()->SetPosition(math::vec::zero);
			root->GetTransform()->SetRotation(math::vec::zero);
			root->GetTransform()->SetScale(math::vec::one);
			root->TransformModified(false);
			root->Update();
			root->ResetGlobalBoundingBoxForAllChilds();
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

RE_GOManager* RE_Prefab::GetPool()
{
	if (!ResourceContainer::inMemory) App::resources->Use(GetMD5());
	return loaded;
}

void RE_Prefab::AssetSave()
{
	//Serialize
	Config prefab_SaveFile(GetAssetPath(), App::fs->GetZipPath());
	JSONNode* prefabNode = prefab_SaveFile.GetRootNode("prefab");

	RE_ResouceAndGOImporter::JsonSerialize(prefabNode, toSave);
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
		loaded = RE_ResouceAndGOImporter::JsonDeserialize(prefabNode);
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
		loaded = RE_ResouceAndGOImporter::BinaryDeserialize(cursor);
	}
	ResourceContainer::inMemory = true;
}

void RE_Prefab::LibrarySave()
{
	uint size = 0;
	char* buffer = RE_ResouceAndGOImporter::BinarySerialize(toSave, &size);
	RE_FileIO toLibrarySave(GetLibraryPath(), App::fs->GetZipPath());
	toLibrarySave.Save(buffer, size);
	DEL_A(buffer);
}

void RE_Prefab::Draw()
{
	if (ImGui::Button("Add to Scene")) App::scene->AddGOPool(GetPool());
}
