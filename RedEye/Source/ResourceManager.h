#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include <map>

class ResourceContainer;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	unsigned int Reference(ResourceContainer* rc);
	bool UnReference(const unsigned intid);
	ResourceContainer* At(const unsigned int id) const;
	unsigned int TotalReferences() const;
	//unsigned int TotalReferenceCount() const;

	typedef std::pair<unsigned int, ResourceContainer*> Resource;
	typedef std::map<unsigned int, ResourceContainer*> ResourceMap;
	typedef ResourceMap::iterator ResourceIter;
	typedef ResourceMap::const_iterator ResourceConstIter;
	
private:
	ResourceMap resources;
	unsigned int reference_count = 1;
};

#endif // !__RESOURCEMANAGER_H__