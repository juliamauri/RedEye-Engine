#include "Resource.h"

#include "RE_Scene.h"

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

void RE_Scene::LoadInMemory()
{
	if (RE_FS->Exists(GetLibraryPath()))
	{
		LibraryLoad();
	}
	else if (RE_FS->Exists(GetAssetPath()))
	{
		AssetLoad();
		LibrarySave(true);
	}
	else RE_LOG_ERROR("Scene %s not found on project", GetName());
}

void RE_Scene::UnloadMemory()
{
	DEL(loaded)
	DEL(toSave)
	ResourceContainer::inMemory = false;
}

void RE_Scene::Import(bool keepInMemory)
{
	AssetLoad(true);
	LibrarySave(true);
	if (!keepInMemory) UnloadMemory();
}

void RE_Scene::Save(RE_ECS_Pool* pool)
{
	if (pool)
	{
		toSave = new RE_ECS_Pool();
		toSave->InsertPool(pool);
		AssetSave();
		LibrarySave();
		if (loaded) DEL(loaded)
		if (toSave) DEL(toSave)
	}
}

void RE_Scene::SetName(const char* _name)
{
	ResourceContainer::SetName(_name);
	eastl::string assetPath("Assets/Scenes/");
	(assetPath += _name) += ".re";
	SetAssetPath(assetPath.c_str());
}

RE_ECS_Pool* RE_Scene::GetPool() { 
	RE_ECS_Pool* ret;
	bool unload = false;
	if (unload = (loaded == nullptr)) LoadInMemory();
	ret = loaded->GetNewPoolFromID(loaded->GetRootUID());
	if (unload) UnloadMemory();
	return ret;
}

void RE_Scene::Draw()
{
	if (ImGui::Button("Load Scene")) RE_SCENE->LoadScene(GetMD5());
}

void RE_Scene::AssetSave()
{
	//Serialize
	Config scene_SaveFile(GetAssetPath());
	RE_Json* scenebNode = scene_SaveFile.GetRootNode("scene");

	if (toSave->TotalGameObjects() > 0) RE_ECS_Importer::JsonSerialize(scenebNode, toSave);
	DEL(scenebNode)

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
	Config jsonLoad(GetAssetPath());

	if (jsonLoad.Load())
	{
		RE_Json* scenebNode = jsonLoad.GetRootNode("scene");
		loaded = RE_ECS_Importer::JsonDeserialize(scenebNode);
		DEL(scenebNode)

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
	RE_FileBuffer binaryLoad(GetLibraryPath());
	if (binaryLoad.Load())
	{
		char* cursor = binaryLoad.GetBuffer();
		loaded = RE_ECS_Importer::BinaryDeserialize(cursor);
	}
	ResourceContainer::inMemory = true;
}

void RE_Scene::LibrarySave(bool fromLoaded)
{
	size_t size = 0;
	char* buffer = RE_ECS_Importer::BinarySerialize((fromLoaded) ? loaded : toSave, &size);
	RE_FileBuffer toLibrarySave(GetLibraryPath());
	toLibrarySave.Save(buffer, size);
	DEL_A(buffer);
}