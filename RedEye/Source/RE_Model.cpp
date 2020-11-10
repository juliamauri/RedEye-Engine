#include "RE_Model.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "ModuleScene.h"
#include "RE_ResouceAndGOImporter.h"
#include "RE_ModelImporter.h"
#include "RE_GOManager.h"
#include "RE_HandleErrors.h"
#include "OutputLog.h"
#include "Globals.h"

#include "assimp\include\postprocess.h"
#include "ImGui/imgui.h"

RE_Model::RE_Model() {}
RE_Model::RE_Model(const char* metaPath) : ResourceContainer(metaPath) {}
RE_Model::~RE_Model() {}

void RE_Model::LoadInMemory()
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
	else RE_LOG_ERROR("Model %s not found on project", GetName());
}

void RE_Model::UnloadMemory()
{
	DEL(loaded);
	ResourceContainer::inMemory = false;
}

void RE_Model::SetAssetPath(const char* originPath)
{
	ResourceContainer::SetAssetPath(originPath);
	eastl::string assetPath(originPath);
	uint l = 0;
	SetName(assetPath.substr(l = assetPath.find_last_of("/") + 1, assetPath.find_last_of(".") - l).c_str());
}

void RE_Model::Import(bool keepInMemory)
{
	AssetLoad();
	LibrarySave();
	if(!keepInMemory) UnloadMemory();
}

RE_GOManager* RE_Model::GetPool()
{
	return loaded;
}

void RE_Model::Draw()
{
	static char* presetsStr[4] = { "None", "TargetRealtime Fast", "TargetRealtime MaxQuality", "TargetRealtime Quality" };
	static char* flagsStr[25] = { "CalcTangentSpace", "JoinIdenticalVertices", "Triangulate", "RemoveComponent", "GenNormals", "GenSmoothNormals", "SplitLargeMeshes", "PreTransformVertices", "LimitBoneWeights", "ValidateDataStructure", "ImproveCacheLocality",  "RemoveRedundantMaterials", "FixInfacingNormals", "SortByPType", "FindDegenerates", "FindInvalidData", "GenUVCoords", "TransformUVCoords", "FindInstances", "OptimizeMeshes", "OptimizeGraph", "FlipUVs", "FlipWindingOrder", "SplitByBoneCount", "Debone" };
	static int currentPresset = modelSettings.GetPresetSelected();

	if (applySave)
	{
		if (ImGui::Button("ReImport and Save"))
		{
			restoreSettings = modelSettings;
			bool neededReLoad = false;
			if (ResourceContainer::inMemory) neededReLoad = true;
			Event::PauseEvents();
			if (neededReLoad) UnloadMemory();
			AssetLoad();
			LibrarySave();
			SaveMeta();
			if(!neededReLoad) UnloadMemory();
			Event::ResumeEvents();
			applySave = false;
		}
		if (ImGui::Button("Restore"))
		{
			modelSettings = restoreSettings;
			currentPresset = -1;
			for (uint i = 0; i < 3; i++)
			{
				if (modelSettings.presets[i])
				{
					currentPresset = i;
					break;
				}
			}
			applySave = false;
		}
	}

	ImGui::Text((currentPresset != -1) ? "Current presset: %s", presetsStr[currentPresset + 1] : "No Presset selected");

	if (ImGui::BeginMenu("Preset Flags"))
	{
		for (int i = -1; i < 3; i++)
		{
			if (ImGui::MenuItem(presetsStr[i + 1]))
			{
				applySave = true;
				currentPresset = i;

				if (currentPresset == -1) for (uint i = 0; i < 3; i++) modelSettings.presets[i] = false;
				else for (uint i = 0; i < 3; i++) modelSettings.presets[i] = (i == (currentPresset));
			}
		}
		ImGui::EndMenu();
	}

	ImGui::Text("Dissable presset for select a couple of flags manually.");
	if (currentPresset == -1)
	{
		if (ImGui::ListBoxHeader("Flags"))
		{
			for (uint i = 0; i < 25; i++)
				if (ImGui::Selectable(flagsStr[i], &modelSettings.flags[i]))
					applySave = true;
		}
		ImGui::ListBoxFooter();
	}

	if (!needReImport && ImGui::Button("Add to Scene"))
	{
		App::handlerrors.StartHandling();

		if (CheckResourcesIsOnAssets())
		{
			App::resources->Use(GetMD5());
			App::scene->AddGOPool(GetPool());
			App::resources->UnUse(GetMD5());
		}
		else
		{
			RE_LOG_ERROR("Missing Resources on Model");
			RE_LOG_SOLUTION("Needed ReImport");
			needReImport = true;
		}

		App::handlerrors.StopAndPresent();
	}
	else if(needReImport && ImGui::Button("ReImport before add"))
	{
		bool neededReLoad = false;
		if (ResourceContainer::inMemory) neededReLoad = true;
		Event::PauseEvents();
		if (neededReLoad) UnloadMemory();
		AssetLoad();
		LibrarySave();
		SaveMeta();
		if (!neededReLoad) UnloadMemory();
		Event::ResumeEvents();
		needReImport = false;
	}

	if (applySave && modelSettings == restoreSettings) applySave = false;
}

void RE_Model::SaveResourceMeta(JSONNode* metaNode)
{
	JSONNode* presets = metaNode->PushJObject("presets");
	for (uint i = 0; i < 3; i++) presets->PushBool(eastl::to_string(i).c_str(), modelSettings.presets[i]);
	DEL(presets);

	JSONNode* flags = metaNode->PushJObject("flags");
	for (uint i = 0; i < 25; i++) flags->PushBool(eastl::to_string(i).c_str(), modelSettings.flags[i]);
	DEL(flags);

	metaNode->PushUInt("MeshesSize", modelSettings.libraryMeshes.size());
	uint count = 0;
	for (const char* mesh : modelSettings.libraryMeshes)
	{
		ResourceContainer* rM = App::resources->At(mesh);
		metaNode->PushString(eastl::to_string(count++).c_str(), rM->GetLibraryPath());
	}
}

void RE_Model::LoadResourceMeta(JSONNode* metaNode)
{
	JSONNode* presets = metaNode->PullJObject("presets");
	for (uint i = 0; i < 3; i++) modelSettings.presets[i] = presets->PullBool(eastl::to_string(i).c_str(), false);
	DEL(presets);

	JSONNode* flags = metaNode->PullJObject("flags");
	for (uint i = 0; i < 25; i++) modelSettings.flags[i] = flags->PullBool(eastl::to_string(i).c_str(), false);
	DEL(flags);

	uint totalMeshes = metaNode->PullUInt("MeshesSize", 0);
	for (uint i = 0; i < totalMeshes; i++)
	{
		eastl::string libraryMesh = metaNode->PullString(eastl::to_string(i).c_str(), "");
		const char* md5 = App::resources->CheckOrFindMeshOnLibrary(libraryMesh.c_str());
		if (md5)
		{
			App::resources->At(md5)->SetAssetPath(GetAssetPath());
			App::resources->At(md5)->SetMetaPath(GetMetaPath());
			modelSettings.libraryMeshes.push_back(md5);
		}
	}
	restoreSettings = modelSettings;
}

void RE_Model::AssetLoad()
{
	RE_FileIO  assetload(GetAssetPath());
	if (assetload.Load())
	{
		loaded = App::modelImporter.ProcessModel(assetload.GetBuffer(), assetload.GetSize(),GetAssetPath(), &modelSettings);
		SetMD5(assetload.GetMd5().c_str());
		eastl::string libraryPath("Library/Models/");
		libraryPath += GetMD5();
		SetLibraryPath(libraryPath.c_str());
		ResourceContainer::inMemory = true;
	}
}

void RE_Model::LibraryLoad()
{
	RE_FileIO binaryLoad(GetLibraryPath());
	if (binaryLoad.Load())
	{
		char* cursor = binaryLoad.GetBuffer();
		loaded = RE_ResouceAndGOImporter::BinaryDeserialize(cursor);
		ResourceContainer::inMemory = true;
	}
}

void RE_Model::LibrarySave()
{
	if (loaded != nullptr)
	{
		uint size = 0;
		char* buffer = RE_ResouceAndGOImporter::BinarySerialize(loaded, &size);

		RE_FileIO toLibrarySave(GetLibraryPath(), App::fs->GetZipPath());
		toLibrarySave.Save(buffer, size);
		DEL_A(buffer);
	}
	else RE_LOG_ERROR("Error to save Model at library because culdn't be loaded from: %s", GetAssetPath());
}

bool RE_Model::CheckResourcesIsOnAssets()
{
	bool ret = false;
	RE_FileIO binaryLoad(GetLibraryPath());

	if (binaryLoad.Load())
	{
		char* cursor = binaryLoad.GetBuffer();
		ret = RE_ResouceAndGOImporter::BinaryCheckResources(cursor);
	}

	return ret;
}

int RE_ModelSettings::GetPresetSelected() const
{
	int ret = -1;
	for (int i = 0; i < 3; i++)
	{
		if (presets[i])
		{
			ret = i;
			break;
		}
	}
	return ret;
}

unsigned int RE_ModelSettings::GetFlags() const
{
	unsigned int ret = 0;
	unsigned int presetFlag = 0;
	bool fromPreset = false;

	for (uint i = 0; i < 3; i++)
	{
		if (presets[i])
		{
			presetFlag = i;
			fromPreset = true;
			break;
		}
	}

	if (fromPreset)
	{
		switch (presetFlag) {
		case 0: ret = aiProcessPreset_TargetRealtime_Fast; break;
		case 1: ret = aiProcessPreset_TargetRealtime_MaxQuality; break;
		case 2: ret = aiProcessPreset_TargetRealtime_Quality; break; }
	}
	else
	{
		eastl::stack<unsigned int> toAdd;
		for (uint i = 0; i < 25; i++) 
			if (flags[i]) toAdd.push(i);
		
		while (!toAdd.empty())
		{
			switch (toAdd.top()) {
			case 0: ret |= aiProcess_CalcTangentSpace; break;
			case 1: ret |= aiProcess_JoinIdenticalVertices; break;
			case 2: ret |= aiProcess_Triangulate; break;
			case 3: ret |= aiProcess_RemoveComponent; break;
			case 4: ret |= aiProcess_GenNormals; break;
			case 5: ret |= aiProcess_GenSmoothNormals; break;
			case 6: ret |= aiProcess_SplitLargeMeshes; break;
			case 7: ret |= aiProcess_PreTransformVertices; break;
			case 8: ret |= aiProcess_LimitBoneWeights; break;
			case 9: ret |= aiProcess_ValidateDataStructure; break;
			case 10: ret |= aiProcess_ImproveCacheLocality; break;
			case 11: ret |= aiProcess_RemoveRedundantMaterials; break;
			case 12: ret |= aiProcess_FixInfacingNormals; break;
			case 13: ret |= aiProcess_SortByPType; break;
			case 14: ret |= aiProcess_FindDegenerates; break;
			case 15: ret |= aiProcess_FindInvalidData; break;
			case 16: ret |= aiProcess_GenUVCoords; break;
			case 17: ret |= aiProcess_TransformUVCoords; break;
			case 18: ret |= aiProcess_FindInstances; break;
			case 19: ret |= aiProcess_OptimizeMeshes; break;
			case 20: ret |= aiProcess_OptimizeGraph; break;
			case 21: ret |= aiProcess_FlipUVs; break;
			case 22: ret |= aiProcess_FlipWindingOrder; break;
			case 23: ret |= aiProcess_SplitByBoneCount; break;
			case 24: ret |= aiProcess_Debone; break; }

			toAdd.pop();
		}
	}

	return ret;
}