#ifndef __MESHMANAGER_H__
#define __MESHMANAGER_H__

#include <map>

class RE_MeshContainer;

typedef std::map<unsigned int, std::pair<RE_MeshContainer*, unsigned int>> MeshMap;
typedef MeshMap::iterator MeshIter;
typedef MeshMap::const_iterator MeshConstIter;

class MeshManager
{
public:
	MeshManager(const char* folderPath);
	~MeshManager();

	void Init(const char* def_shader = nullptr);

	unsigned int LoadMesh(const char* path, const bool dropped = false);
	bool UnReference(const unsigned int mesh_id);
	RE_MeshContainer* operator[](const unsigned int mesh_id) const;
	unsigned int TotalMeshes() const;

	unsigned int GetLoadedMesh(const char* path, const bool from_drop = false) const;

	void DrawMesh(const unsigned int reference);

private:

	unsigned int AddMesh(RE_MeshContainer* mesh);
	
private:

	const char* folderPath = nullptr;
	unsigned int default_shader = 0;
	MeshMap meshes;
};


/*class ResourceContainer;

class ResourceManager WIP!
{
public:
ResourceManager(const char* folderPath) : folderPath(folderPath) {}
virtual ~ResourceManager() {}

virtual void Init(const char* def_shader = nullptr) {}
virtual void Use(const unsigned int reference);

unsigned int LoadToMap(const char* path, const bool dropped = false);
bool UnReference(const unsigned intid);

ResourceContainer* operator[](const unsigned int id) const;
unsigned int TotalLoaded() const;
unsigned int GetLoaded(const char* path, const bool from_drop = false) const;

typedef std::map<unsigned int, std::pair<ResourceContainer*, unsigned int>> ResourceMap;
typedef MeshMap::iterator ResourceIter;
typedef MeshMap::const_iterator ResourceConstIter;

private:

const char* folderPath = nullptr;
ResourceMap resources;
};*/

#endif // !__MESHMANAGER_H__