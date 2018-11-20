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
		if (resource.first.compare(md5) == 0)
		{
			char * writable = new char[resource.first.size() + 1];
			std::copy(resource.first.begin(), resource.first.end(), writable);
			writable[resource.first.size()] = '\0';
			ret = writable;
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
		if (std::string(resource_it.second->GetMD5()).compare(resource) == 0)
		{
			isLoaded = true;
			break;
		}
	}

	std::string path_library("Library/");

	if (!isLoaded)
	{
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
		if (resource.first.compare(md5) == 0)
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