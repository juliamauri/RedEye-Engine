#include "ResourceManager.h"

#include "Application.h"
#include "Texture2DManager.h"
#include "MeshManager.h"
#include "FileSystem.h"
#include "Globals.h"
#include "OutputLog.h"
#include "SDL2\include\SDL_assert.h"

ResourceManager::ResourceManager()
{}

ResourceManager::~ResourceManager()
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

const char* ResourceManager::Reference(ResourceContainer* rc)
{
	resources.insert(Resource(rc->GetMD5(), rc));
	return rc->GetMD5();
}

const char * ResourceManager::IsReference(const char * md5)
{
	const char* ret = nullptr;

	for (auto resource : resources)
	{
		if (std::strcmp(resource.first, md5) == 0)
		{
			ret = resource.first;
			break;
		}
	}

	return ret;
}

void ResourceManager::CheckFileLoaded(const char * filepath, const char* resource, Resource_Type type)
{
	bool isLoaded = false;
	for (auto resource_it : resources)
	{
		if (std::strcmp(resource_it.second->GetMD5(),resource) == 0)
		{
			isLoaded = true;
			break;
		}
	}

	if (!isLoaded)
	{
		std::string path_library("Library/");

		switch (type)
		{
		case R_TEXTURE:
			path_library += "Images/";
			path_library += resource;
			path_library += ".eye";
			if(App->fs->Exists(path_library.c_str()))
				App->textures->LoadTexture2D(path_library.c_str(), true, filepath);
			else
				App->textures->LoadTexture2D(filepath);
			break;
		case R_MESH:
			path_library += "Meshes/";
			path_library += resource;
			path_library += ".red";
			if (App->fs->Exists(path_library.c_str()))
				App->meshes->LoadDirectMesh(path_library.c_str(), resource, filepath);
			else
				App->meshes->LoadMesh(filepath);
			break;
		default:
			break;
		}
	}
}

bool ResourceManager::UnReference(const unsigned intid)
{
	bool ret = false;
	/*
	if (intid > 0)
	{
		ResourceIter it = resources.find(intid);
		if (it != resources.end())
		{
			// Release Mesh
			SDL_assert(it->second != nullptr);
			DEL(it->second);
			resources.erase(it);
			ret = true;
		}
	}
	*/
	return ret;
}

ResourceContainer* ResourceManager::At(const char* md5) const
{
	ResourceContainer* ret = nullptr;

	for (auto resource : resources)
	{
		if (std::strcmp(resource.first,md5) == 0)
		{
			ret = resource.second;
			break;
		}
	}

	return ret;
}

unsigned int ResourceManager::TotalReferences() const
{
	return resources.size();
}
std::vector<ResourceContainer*> ResourceManager::GetResourcesByType(Resource_Type type)
{
	std::vector<ResourceContainer*> ret;
	for (auto resource : resources)
	{
		if (resource.second->GetType() == type)
			ret.push_back(resource.second);
	}
	return ret;
}
/*
unsigned int ResourceManager::TotalReferenceCount() const
{
	unsigned int ret = 0;

	ResourceConstIter it = resources.begin();
	for (; it != resources.end(); it++)
		ret += it->second.second;

	return ret;
}
*/