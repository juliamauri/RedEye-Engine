#include "ResourceManager.h"

#include "Resource.h"
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
		resources.erase(resources.begin());
	}
}

const char* ResourceManager::Reference(ResourceContainer* rc)
{
	resources.insert(Resource(rc->GetMD5(), rc));
	return rc->GetMD5()->c_str();
}

const char * ResourceManager::IsReference(const char * md5)
{
	const char* ret = nullptr;

	for (auto resource : resources)
	{
		if (resource.first->compare(md5) == 0)
			ret = resource.first->c_str();
	}

	return ret;
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
		if (resource.first->compare(md5) == 0)
			ret = resource.second;
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