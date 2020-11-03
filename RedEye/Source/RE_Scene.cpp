#include "RE_Scene.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "ModuleScene.h"
#include "RE_ResourceManager.h"
#include "RE_GOManager.h"
#include "RE_ResouceAndGOImporter.h"
#include  "OutputLog.h"

#include "ImGui/imgui.h"

RE_Scene::RE_Scene() {}
RE_Scene::RE_Scene(const char* metaPath) : ResourceContainer(metaPath) {}
RE_Scene::~RE_Scene() {}

void RE_Scene::LoadInMemory()
{
	if (App::fs->Exists(GetLibraryPath()))
	{
		LibraryLoad();
	}
	else if (App::fs->Exists(GetAssetPath()))
	{
		AssetLoad();
		LibrarySave(true);
	}
	else RE_LOG_ERROR("Scene %s not found on project", GetName());
}

void RE_Scene::UnloadMemory()
{
	if (loaded) DEL(loaded);
	if (toSave) DEL(toSave);
	loaded = nullptr;
	toSave = nullptr;
	ResourceContainer::inMemory = false;
}

void RE_Scene::Import(bool keepInMemory)
{
	AssetLoad(true);
	LibrarySave(true);
	if (!keepInMemory) UnloadMemory();
}

void RE_Scene::Save(RE_GOManager* pool)
{
	if (pool)
	{
		toSave = new RE_GOManager();
		toSave->InsertPool(pool);
		AssetSave();
		LibrarySave();
		if (loaded) DEL(loaded);
		if (toSave) DEL(toSave);
	}
}

void RE_Scene::SetName(const char* _name)
{
	ResourceContainer::SetName(_name);
	eastl::string assetPath("Assets/Scenes/");
	(assetPath += _name) += ".re";
	SetAssetPath(assetPath.c_str());
}

RE_GOManager* RE_Scene::GetPool() { return loaded; }

void RE_Scene::Draw()
{
	if (ImGui::Button("Load Scene")) App::scene->LoadScene(GetMD5());
}

void RE_Scene::AssetSave()
{
	//Serialize
	Config scene_SaveFile(GetAssetPath(), App::fs->GetZipPath());
	JSONNode* scenebNode = scene_SaveFile.GetRootNode("scene");

	if (toSave->TotalGameObjects() > 0) RE_ResouceAndGOImporter::JsonSerialize(scenebNode, toSave);
	DEL(scenebNode);

	//Setting LibraryPath and MD5
	eastl::string md5 = scene_SaveFile.GetMd5();
	SetMD5(md5.c_str());
	eastl::string libraryPath("Library/Scenes/");
	libraryPath += md5;
	SetLibraryPath(libraryPath.c_str());

	//Save
	scene_SaveFile.Save();
}

void RE_Scene::AssetLoad(bool generateLibraryPath)
{
	Config jsonLoad(GetAssetPath(), App::fs->GetZipPath());

	if (jsonLoad.Load())
	{
		JSONNode* scenebNode = jsonLoad.GetRootNode("scene");
		loaded = RE_ResouceAndGOImporter::JsonDeserialize(scenebNode);
		DEL(scenebNode);

		if (generateLibraryPath)
		{
			eastl::string md5 = jsonLoad.GetMd5();
			SetMD5(md5.c_str());
			SetLibraryPath(("Library/Scenes/" + md5).c_str());
		}
	}

	ResourceContainer::inMemory = true;
}

void RE_Scene::LibraryLoad()
{
	RE_FileIO binaryLoad(GetLibraryPath());
	if (binaryLoad.Load())
	{
		char* cursor = binaryLoad.GetBuffer();
		loaded = RE_ResouceAndGOImporter::BinaryDeserialize(cursor);
	}
	ResourceContainer::inMemory = true;
}

void RE_Scene::LibrarySave(bool fromLoaded)
{
	uint size = 0;
	char* buffer = RE_ResouceAndGOImporter::BinarySerialize((fromLoaded) ? loaded : toSave, &size);
	RE_FileIO toLibrarySave(GetLibraryPath(), App::fs->GetZipPath());
	toLibrarySave.Save(buffer, size);
	DEL_A(buffer);
}