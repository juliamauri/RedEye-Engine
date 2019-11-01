#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include <map>
#include <vector>

class ResourceContainer;
#include "Resource.h"

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	const char* Reference(ResourceContainer* rc);
	const char* IsReference(const char* md5);
	const char* CheckFileLoaded(const char* filepath, const char* resource, Resource_Type type);
	bool UnReference(const unsigned intid);
	ResourceContainer* At(const char* md5) const;
	unsigned int TotalReferences() const;
	//unsigned int TotalReferenceCount() const;
	std::vector<ResourceContainer*> GetResourcesByType(Resource_Type type);

	const char* FindMD5ByMETAPath(const char* metaPath, Resource_Type type = Resource_Type::R_UNDEFINED);
	const char* FindMD5ByAssetsPath(const char* assetsPath, Resource_Type type = Resource_Type::R_UNDEFINED);



	typedef std::pair<const char*, ResourceContainer*> Resource;
	typedef std::map<const char*, ResourceContainer*> ResourceMap;
	typedef ResourceMap::iterator ResourceIter;
	typedef ResourceMap::const_iterator ResourceConstIter;
	
private:
	ResourceMap resources;
};

#endif // !__RESOURCEMANAGER_H__