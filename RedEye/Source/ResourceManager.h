#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include <map>

class ResourceContainer;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	const char* Reference(ResourceContainer* rc);
	const char* IsReference(const char* md5);
	bool UnReference(const unsigned intid);
	ResourceContainer* At(const char* md5) const;
	unsigned int TotalReferences() const;
	//unsigned int TotalReferenceCount() const;

	typedef std::pair<std::string*, ResourceContainer*> Resource;
	typedef std::map<std::string*, ResourceContainer*> ResourceMap;
	typedef ResourceMap::iterator ResourceIter;
	typedef ResourceMap::const_iterator ResourceConstIter;
	
private:
	ResourceMap resources;
};

#endif // !__RESOURCEMANAGER_H__