#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include <map>

enum Resource_Type : short unsigned int
{
	R_UNDEFINED = 0x00,
	R_SHADER,
	R_PRIMITIVE,
	R_TEXTURE,
	R_MESH
};

class ResourceContainer;

class ResourceManager
{
public:
	ResourceManager(Resource_Type type);
	virtual ~ResourceManager();

	bool UnReference(const unsigned intid);
	ResourceContainer* operator[](const unsigned int id) const;
	unsigned int TotalReferences() const;
	unsigned int TotalReferenceCount() const;

	Resource_Type GetType() const;

	typedef std::map<unsigned int, std::pair<ResourceContainer*, unsigned int>> ResourceMap;
	typedef ResourceMap::iterator ResourceIter;
	typedef ResourceMap::const_iterator ResourceConstIter;

protected:

	ResourceMap resources;

private:

	Resource_Type type = R_UNDEFINED;

};

#endif // !__RESOURCEMANAGER_H__