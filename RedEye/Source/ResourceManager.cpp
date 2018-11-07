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

unsigned int ResourceManager::Reference(ResourceContainer* rc)
{
	rc->SetID(reference_count);
	resources.insert(Resource(reference_count, rc));
	return reference_count++;
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

ResourceContainer* ResourceManager::At(const unsigned int id) const
{
	ResourceConstIter it = resources.find(id);
	return it != resources.end() ? it->second : nullptr;
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