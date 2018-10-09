#include "ResourceManager.h"

#include "Resource.h"
#include "Globals.h"
#include "OutputLog.h"
#include "SDL2\include\SDL_assert.h"


ResourceManager::ResourceManager(Resource_Type type) : type(type)
{}

ResourceManager::~ResourceManager()
{
	while (!resources.empty())
	{
		LOG("WARNING: Deleating Unreferenced Resource: %s (from %s)",
			resources.begin()->second.first->GetName(),
			resources.begin()->second.first->GetOrigin());
		resources.erase(resources.begin());
	}
}

bool ResourceManager::UnReference(const unsigned intid)
{
	bool ret = false;

	if (intid > 0)
	{
		ResourceIter it = resources.find(intid);
		if (it != resources.end())
		{
			it->second.second--;

			if (it->second.second == 0)
			{
				// Release Mesh
				SDL_assert(it->second.first != nullptr);
				DEL(it->second.first);
				resources.erase(it);
			}

			ret = true;
		}
	}

	return ret;
}

ResourceContainer * ResourceManager::operator[](const unsigned int id) const
{
	ResourceConstIter it = resources.find(id);
	return it != resources.end() ? it->second.first : nullptr;
}

unsigned int ResourceManager::TotalReferences() const
{
	return resources.size();
}

unsigned int ResourceManager::TotalReferenceCount() const
{
	unsigned int ret = 0;

	ResourceConstIter it = resources.begin();
	for (; it != resources.end(); it++)
		ret += it->second.second;

	return ret;
}

Resource_Type ResourceManager::GetType() const
{
	return type;
}
