#include "RE_ResourceManager.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_TextureImporter.h"
#include "RE_ModelImporter.h"

#include "RE_Material.h"
#include "RE_Shader.h"
#include "RE_Prefab.h"
#include "RE_Model.h"
#include "RE_Scene.h"
#include "RE_SkyBox.h"
#include "RE_Texture.h"

#include "Globals.h"
#include "OutputLog.h"
#include "SDL2\include\SDL_assert.h"

#include <string>

RE_ResourceManager::RE_ResourceManager()
{}

RE_ResourceManager::~RE_ResourceManager()
{
	while (!resources.empty())
	{
		//LOG("WARNING: Deleating Unreferenced Resource: %s (from %s)",
			//resources.begin()->second.first->GetName(),
			//resources.begin()->second.first->GetOrigin());
		DEL(resources.begin()->second);
		resources.erase(resources.begin());
	}
}

const char* RE_ResourceManager::Reference(ResourceContainer* rc)
{
	std::string resourceName;
	switch (rc->GetType())
	{
	case Resource_Type::R_TEXTURE:
		resourceName = "texture";
		break;
	case Resource_Type::R_MATERIAL:
		resourceName = "material";
		break;
	case Resource_Type::R_MESH:
		resourceName = "mesh";
		break;
	case Resource_Type::R_PREFAB:
		resourceName = "prefab";
		break;
	case Resource_Type::R_PRIMITIVE:
		resourceName = "primitive";
		break;
	case Resource_Type::R_SHADER:
		resourceName = "shader";
		break;
	case Resource_Type::R_MODEL:
		resourceName = "model";
		break;
	case Resource_Type::R_UNDEFINED:
		resourceName = "undefined";
		break;
	}
	LOG("Referencing the %s %s resource from %s\nAsset file: %s\nmd5 generated: %s\n", rc->GetName(), resourceName.c_str(), rc->GetAssetPath(), rc->GetLibraryPath(), rc->GetMD5());
	resources.insert(Resource(rc->GetMD5(), rc));
	resourcesCounter.insert(ResourceCounter(rc->GetMD5(), (rc->isInMemory()) ? 1 : 0));
	return rc->GetMD5();
}

void RE_ResourceManager::Use(const char* resMD5)
{
	if (resourcesCounter.at(resMD5) == 0) resources.at(resMD5)->LoadInMemory();
	resourcesCounter.at(resMD5)++;
}

void RE_ResourceManager::UnUse(const char* resMD5)
{
	if (--resourcesCounter.at(resMD5) == 0) resources.at(resMD5)->UnloadMemory();
	else if (resourcesCounter.at(resMD5) < 0) {
		LOG_WARNING("UnUse of resource already with no uses. Resource %s.",resources.at(resMD5)->GetName());
		if(resources.at(resMD5)->isInMemory()) resources.at(resMD5)->UnloadMemory();
		resourcesCounter.at(resMD5) = 0;
	}
}

ResourceContainer* RE_ResourceManager::At(const char* md5) const
{
	return resources.at(md5);
}

const char* RE_ResourceManager::ReferenceByMeta(const char* metaPath, Resource_Type type)
{
	const char* retMD5 = nullptr;
	switch (type)
	{
	case R_SHADER:
		retMD5 = Reference((ResourceContainer*)new RE_Shader(metaPath));
		break;
	case R_TEXTURE:
		retMD5 = Reference((ResourceContainer*)new RE_Texture(metaPath));
		break;
	case R_PREFAB:
		retMD5 = Reference((ResourceContainer*)new RE_Prefab(metaPath));
		break;
	case R_SKYBOX:
		retMD5 = Reference((ResourceContainer*)new RE_SkyBox(metaPath));
		break;
	case R_MATERIAL:
		retMD5 = Reference((ResourceContainer*)new RE_Material(metaPath));
		break;
	case R_MODEL:
		retMD5 = Reference((ResourceContainer*)new RE_Model(metaPath));
		break;
	}
	return retMD5;
}

unsigned int RE_ResourceManager::TotalReferences() const
{
	return resources.size();
}
const char* RE_ResourceManager::ImportModel(const char* assetPath)
{
	std::string path(assetPath);
	std::string filename = path.substr(path.find_last_of("/") + 1);
	std::string name = filename.substr(0, filename.find_last_of(".") - 1);
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Model* newModel = new RE_Model();
	newModel->SetName(name.c_str());
	newModel->SetAssetPath(assetPath);
	newModel->SetType(Resource_Type::R_MODEL);
	newModel->Import(false);

	return Reference(newModel);
}
const char* RE_ResourceManager::ImportTexture(const char* assetPath)
{
	std::string path(assetPath);
	std::string filename = path.substr(path.find_last_of("/") + 1);
	std::string name = filename.substr(0, filename.find_last_of(".") - 1);
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Texture* newTexture = new RE_Texture();
	newTexture->SetName(name.c_str());
	newTexture->SetAssetPath(assetPath);
	newTexture->SetType(Resource_Type::R_TEXTURE);
	newTexture->Import(false);

	return Reference(newTexture);
}
std::vector<ResourceContainer*> RE_ResourceManager::GetResourcesByType(Resource_Type type)
{
	std::vector<ResourceContainer*> ret;
	for (auto resource : resources)
	{
		if (resource.second->GetType() == type)
			ret.push_back(resource.second);
	}
	return ret;
}

const char* RE_ResourceManager::IsReference(const char* md5, Resource_Type type)
{
	const char* ret = nullptr;
	if (type != Resource_Type::R_UNDEFINED) {
		for (auto resource : resources)
		{
			if (std::strcmp(resource.second->GetMD5(), md5) == 0)
				ret = resource.first;
		}
	}
	else
	{
		std::vector<ResourceContainer*> tResources = GetResourcesByType(type);
		for (auto resource : tResources)
		{
			if (std::strcmp(resource->GetMD5(), md5) == 0)
				ret = resource->GetMD5();
		}
	}
	return ret;
}

const char * RE_ResourceManager::FindMD5ByMETAPath(const char * metaPath, Resource_Type type)
{
	const char* ret = nullptr;
	if (type != Resource_Type::R_UNDEFINED) {
		for (auto resource : resources)
		{
			if (std::strcmp(resource.second->GetMetaPath(), metaPath) == 0)
				ret = resource.first;
		}
	}
	else
	{
		std::vector<ResourceContainer*> tResources = GetResourcesByType(type);
		for (auto resource : tResources)
		{
			if (std::strcmp(resource->GetMetaPath(), metaPath) == 0)
				ret = resource->GetMD5();
		}
	}
	return ret;
}

const char* RE_ResourceManager::FindMD5ByLibraryPath(const char* libraryPath, Resource_Type type)
{
	const char* ret = nullptr;
	if (type != Resource_Type::R_UNDEFINED) {
		for (auto resource : resources)
		{
			if (std::strcmp(resource.second->GetLibraryPath(), libraryPath) == 0)
				ret = resource.first;
		}
	}
	else
	{
		std::vector<ResourceContainer*> tResources = GetResourcesByType(type);
		for (auto resource : tResources)
		{
			if (std::strcmp(resource->GetLibraryPath(), libraryPath) == 0)
				ret = resource->GetMD5();
		}
	}
	return ret;
}

const char * RE_ResourceManager::FindMD5ByAssetsPath(const char * assetsPath, Resource_Type type)
{
	const char* ret = nullptr;
	if (type != Resource_Type::R_UNDEFINED) {
		for (auto resource : resources)
		{
			if (std::strcmp(resource.second->GetAssetPath(), assetsPath) == 0)
				ret = resource.first;
		}
	}
	else
	{
		std::vector<ResourceContainer*> tResources = GetResourcesByType(type);
		for (auto resource : tResources)
		{
			if (std::strcmp(resource->GetAssetPath(), assetsPath) == 0)
				ret = resource->GetMD5();
		}
	}
	return ret;
}

const char* RE_ResourceManager::ImportMaterial(const char* assetPath)
{
	std::string path(assetPath);
	std::string filename = path.substr(path.find_last_of("/") + 1);
	std::string name = filename.substr(0, filename.find_last_of(".") - 1);
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Material* newMaterial = new RE_Material();
	newMaterial->SetName(name.c_str());
	newMaterial->SetAssetPath(assetPath);
	newMaterial->SetType(Resource_Type::R_TEXTURE);
	newMaterial->Import(false);

	return Reference(newMaterial);
}

const char* RE_ResourceManager::ImportSkyBox(const char* assetPath)
{
	std::string path(assetPath);
	std::string filename = path.substr(path.find_last_of("/") + 1);
	std::string name = filename.substr(0, filename.find_last_of(".") - 1);
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_SkyBox* newSkyBox = new RE_SkyBox();
	newSkyBox->SetName(name.c_str());
	newSkyBox->SetAssetPath(assetPath);
	newSkyBox->SetType(Resource_Type::R_SKYBOX);
	newSkyBox->Import(false);

	return Reference(newSkyBox);
}

const char* RE_ResourceManager::ImportPrefab(const char* assetPath)
{
	std::string path(assetPath);
	std::string filename = path.substr(path.find_last_of("/") + 1);
	std::string name = filename.substr(0, filename.find_last_of(".") - 1);
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Prefab* newSkyBox = new RE_Prefab();
	newSkyBox->SetName(name.c_str());
	newSkyBox->SetAssetPath(assetPath);
	newSkyBox->SetType(Resource_Type::R_PREFAB);
	newSkyBox->Import(false);

	return Reference(newSkyBox);
}

const char* RE_ResourceManager::ImportScene(const char* assetPath)
{
	std::string path(assetPath);
	std::string filename = path.substr(path.find_last_of("/") + 1);
	std::string name = filename.substr(0, filename.find_last_of(".") - 1);
	std::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Scene* newSkyBox = new RE_Scene();
	newSkyBox->SetName(name.c_str());
	newSkyBox->SetAssetPath(assetPath);
	newSkyBox->SetType(Resource_Type::R_SCENE);
	newSkyBox->Import(false);

	return Reference(newSkyBox);
}

unsigned int RE_ResourceManager::TotalReferenceCount(const char* resMD5) const
{
	return resourcesCounter.at(resMD5);
}