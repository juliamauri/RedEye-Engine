#include "RE_Scene.h"

#include "Application.h"
#include "RE_FileSystem.h"

#include "RE_GameObject.h"
#include "RE_ResouceAndGOImporter.h"

#include  "OutputLog.h"

#include "ImGui/imgui.h"

RE_Scene::RE_Scene() { }

RE_Scene::RE_Scene(const char* metaPath) : ResourceContainer(metaPath) { }

RE_Scene::~RE_Scene() { }

void RE_Scene::LoadInMemory()
{
	if (App->fs->Exists(GetLibraryPath()))
		LibraryLoad();
	else if (App->fs->Exists(GetAssetPath())) {
		AssetLoad();
		LibrarySave();
	}
	else {
		LOG_ERROR("Scene %s not found on project", GetName());
	}
}

void RE_Scene::UnloadMemory()
{
	if (loaded) DEL(loaded);
	ResourceContainer::inMemory = false;
}

void RE_Scene::Import(bool keepInMemory)
{
	AssetLoad(true);
	LibrarySave();
	if (!keepInMemory) UnloadMemory();
}

void RE_Scene::Save(RE_GameObject* go)
{
	if (go) {
		toSave = go;
		AssetSave();
		LibrarySave();
	}
}

void RE_Scene::SetName(const char* _name)
{
	ResourceContainer::SetName(_name);

	std::string assetPath("Assets/Scenes/");
	assetPath += _name;
	assetPath += ".re";
	SetAssetPath(assetPath.c_str());
}

RE_GameObject* RE_Scene::GetRoot()
{
	bool neededUnload = false;
	if (neededUnload = !ResourceContainer::inMemory) LoadInMemory();
	RE_GameObject* ret = (loaded) ? new RE_GameObject(*loaded) : nullptr;
	if (neededUnload) UnloadMemory();
	return ret;
}

void RE_Scene::Draw()
{
	if (ImGui::Button("Load Scene")) {
		//TODO Reset Scene
	}
}

void RE_Scene::AssetSave()
{
	//Serialize
	Config scene_SaveFile(GetAssetPath(), App->fs->GetZipPath());
	JSONNode* scenebNode = scene_SaveFile.GetRootNode("scene");

	RE_ResouceAndGOImporter::JsonSerialize(scenebNode, toSave);
	DEL(scenebNode);

	//Setting LibraryPath and MD5
	std::string md5 = scene_SaveFile.GetMd5();
	SetMD5(md5.c_str());
	std::string libraryPath("Library/Scene/");
	libraryPath += md5;
	SetLibraryPath(libraryPath.c_str());

	//Save
	scene_SaveFile.Save();
}

void RE_Scene::AssetLoad(bool generateLibraryPath)
{
	Config jsonLoad(GetAssetPath(), App->fs->GetZipPath());

	if (jsonLoad.Load()) {
		JSONNode* scenebNode = jsonLoad.GetRootNode("scene");
		loaded = RE_ResouceAndGOImporter::JsonDeserialize(scenebNode);
		DEL(scenebNode);

		if (generateLibraryPath) {
			std::string md5 = jsonLoad.GetMd5();
			SetMD5(md5.c_str());
			std::string libraryPath("Library/Scene/");
			libraryPath += md5;
			SetLibraryPath(libraryPath.c_str());
		}
	}

	ResourceContainer::inMemory = true;
}

void RE_Scene::LibraryLoad()
{
	RE_FileIO binaryLoad(GetLibraryPath());

	if (binaryLoad.Load()) {
		char* cursor = binaryLoad.GetBuffer();
		loaded = RE_ResouceAndGOImporter::BinaryDeserialize(cursor);
	}
	ResourceContainer::inMemory = true;
}

void RE_Scene::LibrarySave()
{
	uint size = 0;
	char* buffer = RE_ResouceAndGOImporter::BinarySerialize(toSave, &size);

	RE_FileIO toLibrarySave(GetLibraryPath(), App->fs->GetZipPath());
	toLibrarySave.Save(buffer, size);
	DEL_A(buffer);
}