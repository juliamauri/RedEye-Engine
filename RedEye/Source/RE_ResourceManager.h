#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include <map>
#include <vector>

#include "Resource.h"

class RE_ResourceManager
{
public:
	RE_ResourceManager();
	~RE_ResourceManager();

	ResourceContainer* At(const char* md5) const;
	const char* Reference(ResourceContainer* rc);
	unsigned int TotalReferences() const;

	unsigned int TotalReferenceCount(const char* resMD5) const;
	void Use(const char* resMD5);
	void UnUse(const char* resMD5);

	std::vector<ResourceContainer*> GetResourcesByType(Resource_Type type);
	const char* IsReference(const char* md5, Resource_Type type = Resource_Type::R_UNDEFINED);
	const char* FindMD5ByMETAPath(const char* metaPath, Resource_Type type = Resource_Type::R_UNDEFINED);
	const char* FindMD5ByLibraryPath(const char* libraryPath, Resource_Type type = Resource_Type::R_UNDEFINED);
	const char* FindMD5ByAssetsPath(const char* assetsPath, Resource_Type type = Resource_Type::R_UNDEFINED);

	typedef std::pair<const char*, ResourceContainer*> Resource;
	typedef std::map<const char*, ResourceContainer*> ResourceMap;
	typedef ResourceMap::iterator ResourceIter;
	typedef ResourceMap::const_iterator ResourceConstIter;
	
	typedef std::pair<const char*, unsigned int> ResourceCounter;
	typedef std::map<const char*, unsigned int> ResourceCounterMap;

private:
	ResourceMap resources;
	ResourceCounterMap resourcesCounter;
};

#endif // !__RESOURCEMANAGER_H__