#include "RE_Model.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"

#include "RE_ResouceAndGOImporter.h"
#include "RE_ModelImporter.h"

#include "RE_GameObject.h"

#include "OutputLog.h"
#include "Globals.h"
#include "assimp\include\postprocess.h"

RE_Model::RE_Model() { }

RE_Model::RE_Model(const char* metaPath) : ResourceContainer(metaPath) { }

RE_Model::~RE_Model() { }

void RE_Model::LoadInMemory()
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

void RE_Model::UnloadMemory()
{
	if (loaded) DEL(loaded);
	ResourceContainer::inMemory = false;
}

void RE_Model::SetAssetPath(const char* originPath)
{
	ResourceContainer::SetAssetPath(originPath);
	std::string assetPath(originPath);
	uint l = 0;
	SetName(assetPath.substr(l = assetPath.find_last_of("/") + 1, assetPath.find_last_of(".") - l).c_str());
}

void RE_Model::Import(bool keepInMemory)
{
	AssetLoad();
	LibrarySave();
	if(!keepInMemory) UnloadMemory();
}

RE_GameObject* RE_Model::GetRoot()
{
	bool neededUnload = false;
	if (neededUnload = !ResourceContainer::inMemory) LoadInMemory();
	RE_GameObject* ret = (loaded) ? new RE_GameObject(*loaded) : nullptr;
	if (neededUnload) UnloadMemory();
	return ret;
}

void RE_Model::Draw()
{
}

void RE_Model::SaveResourceMeta(JSONNode* metaNode)
{
	metaNode->PushUInt("MeshesSize", modelSettings.libraryMeshes.size());
	uint count = 0;
	for (const char* mesh : modelSettings.libraryMeshes) {
		ResourceContainer* rM = App->resources->At(mesh);
		metaNode->PushString(std::to_string(count++).c_str(), rM->GetLibraryPath());
	}
}

void RE_Model::LoadResourceMeta(JSONNode* metaNode)
{
	uint totalMeshes = metaNode->PullUInt("MeshesSize", 0);
	for (uint i = 0; i < totalMeshes; i++) {
		std::string libraryMesh = metaNode->PullString(std::to_string(i).c_str(), "");
		const char* md5 = App->resources->CheckOrFindMeshOnLibrary(libraryMesh.c_str());
		modelSettings.libraryMeshes.push_back(md5);
	}
}

void RE_Model::AssetLoad()
{
	RE_FileIO  assetload(GetAssetPath());

	if (assetload.Load()) {
		loaded = App->modelImporter->ProcessModel(assetload.GetBuffer(), assetload.GetSize(),GetAssetPath(), &modelSettings);

		SetMD5(assetload.GetMd5().c_str());
		std::string libraryPath("Library/Models/");
		libraryPath += GetMD5();
		SetLibraryPath(libraryPath.c_str());

		ResourceContainer::inMemory = true;
	}
}

void RE_Model::LibraryLoad()
{
	RE_FileIO binaryLoad(GetLibraryPath());

	if (binaryLoad.Load()) {
		char* cursor = binaryLoad.GetBuffer();
		loaded = RE_ResouceAndGOImporter::BinaryDeserialize(cursor);

		ResourceContainer::inMemory = true;
	}
}

void RE_Model::LibrarySave()
{
	uint size = 0;
	char* buffer = RE_ResouceAndGOImporter::BinarySerialize(loaded, &size);

	RE_FileIO toLibrarySave(GetLibraryPath(), App->fs->GetZipPath());
	toLibrarySave.Save(buffer, size);
	DEL_A(buffer);
}

unsigned int RE_ModelSettings::GetFlags() const
{
	return aiProcessPreset_TargetRealtime_Quality;
}