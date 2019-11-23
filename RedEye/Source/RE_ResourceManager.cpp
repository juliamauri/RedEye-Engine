#include "RE_ResourceManager.h"

#include "Application.h"
#include "RE_TextureImporter.h"
#include "RE_ModelImporter.h"
#include "RE_Material.h"
#include "RE_Prefab.h"
#include "FileSystem.h"
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
	case Resource_Type::R_INTERNALPREFAB:
		resourceName = "internal prefab";
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

unsigned int RE_ResourceManager::TotalReferences() const
{
	return resources.size();
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

unsigned int RE_ResourceManager::TotalReferenceCount(const char* resMD5) const
{
	return resourcesCounter.at(resMD5);
}