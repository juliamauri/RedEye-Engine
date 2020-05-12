#include "RE_Prefab.h"

#include "Application.h"
#include "ModuleScene.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_FileSystem.h"

#include  "OutputLog.h"

#include "md5.h"
#include "RE_ResouceAndGOImporter.h"

#include "Globals.h"

#include "ImGui/imgui.h"

#include <EASTL/map.h>


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
		LOG_ERROR("Prefab %s not found on project", GetName());
	}
}

void RE_Prefab::UnloadMemory()
{
	if (loaded) DEL(loaded);
	ResourceContainer::inMemory = false;
}

void RE_Prefab::Import(bool keepInMemory)
{
	AssetLoad();
	LibrarySave();
	if (!keepInMemory) UnloadMemory();
}

void RE_Prefab::Save(RE_GameObject* go, bool rootidentity, bool keepInMemory)
{
	if (go) {
		loaded = toSave = new RE_GameObject(*go);
		if (rootidentity) {
			loaded->GetTransform()->SetPosition(math::vec::zero);
			loaded->GetTransform()->SetRotation(math::vec::zero);
			loaded->GetTransform()->SetScale(math::vec::one);
			loaded->TransformModified(false);
			loaded->Update();
			loaded->ResetGlobalBoundingBoxForAllChilds();
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
	Config jsonLoad(GetAssetPath(), App->fs->GetZipPath());

	if (jsonLoad.Load()) {
		JSONNode* prefabNode = jsonLoad.GetRootNode("prefab");
		loaded = RE_ResouceAndGOImporter::JsonDeserialize(prefabNode);
		DEL(prefabNode);

		if (generateLibraryPath) {
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
	DEL_A(buffer);
}

void RE_Prefab::Draw()
{
	if (ImGui::Button("Add to Scene")) {
		App->scene->AddGameobject(GetRoot());
	}
}
