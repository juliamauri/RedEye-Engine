#include "RE_Prefab.h"
#include "Globals.h"

#include "Application.h"
#include "RE_GameObject.h"
#include "FileSystem.h"

#include  "OutputLog.h"

#include "md5.h"
#include "ResourceManager.h"
#include "RE_ResouceAndGOImporter.h"

#include <map>


RE_Prefab::RE_Prefab() { }

RE_Prefab::RE_Prefab(const char* metaPath) : ResourceContainer(metaPath) { }

RE_Prefab::~RE_Prefab() { }

void RE_Prefab::LoadInMemory()
{
	if (App->fs->Exists(GetLibraryPath()))
		LibraryLoad();
	else if (App->fs->Exists(GetAssetPath())) {
		AssetLoad();
		LibrarySave();
	}
	else {
		LOG_ERROR("SkyBox %s not found on project", GetName());
	}
}

void RE_Prefab::UnloadMemory()
{
	if (loaded) DEL(loaded);
	ResourceContainer::inMemory = false;
}

void RE_Prefab::Save(RE_GameObject* go)
{
	if (go) {
		toSave = go;
		AssetSave();
		LibrarySave();
	}
}

void RE_Prefab::SetName(const char* _name)
{
	ResourceContainer::SetName(_name);

	std::string assetPath("Assets/Prefabs/");
	assetPath += _name;
	assetPath += ".refab";
	SetAssetPath(assetPath.c_str());
}

RE_GameObject * RE_Prefab::GetRoot()
{
	bool neededUnload = false;
	if (neededUnload = !ResourceContainer::inMemory) LoadInMemory();
	RE_GameObject* ret = (loaded) ? new RE_GameObject(*loaded) : nullptr;
	if (neededUnload) UnloadMemory();
	return ret;
}

void RE_Prefab::AssetSave()
{
	//Serialize
	Config prefab_SaveFile(GetAssetPath(), App->fs->GetZipPath());
	JSONNode* prefabNode = prefab_SaveFile.GetRootNode("prefab");

	RE_ResouceAndGOImporter::JsonSerialize(prefabNode, toSave);
	DEL(prefabNode);

	//Setting LibraryPath and MD5
	std::string md5 = prefab_SaveFile.GetMd5();
	SetMD5(md5.c_str());
	std::string libraryPath("Library/Prefabs/");
	libraryPath += md5;
	SetLibraryPath(libraryPath.c_str());

	//Save
	prefab_SaveFile.Save();
}

void RE_Prefab::AssetLoad()
{
	Config jsonLoad(GetAssetPath(), App->fs->GetZipPath());

	if (jsonLoad.Load()) {
		JSONNode* prefabNode = jsonLoad.GetRootNode("prefab");
		loaded = RE_ResouceAndGOImporter::JsonDeserialize(prefabNode);
		DEL(prefabNode);
	}

	ResourceContainer::inMemory = true;
}

void RE_Prefab::LibraryLoad()
{
	RE_FileIO binaryLoad(GetLibraryPath());

	if (binaryLoad.Load()) {
		char* cursor = binaryLoad.GetBuffer();
		loaded = RE_ResouceAndGOImporter::BinaryDeserialize(cursor);
	}
	ResourceContainer::inMemory = true;
}

void RE_Prefab::LibrarySave()
{
	uint size = 0;
	char* buffer = RE_ResouceAndGOImporter::BinarySerialize(toSave, &size);

	RE_FileIO toLibrarySave(GetLibraryPath(), App->fs->GetZipPath());
	toLibrarySave.Save(buffer, size);
}