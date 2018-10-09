#ifndef __MESHMANAGER_H__
#define __MESHMANAGER_H__

#include "ResourceManager.h"

class RE_MeshContainer;

class MeshManager : ResourceManager
{
public:
	MeshManager(const char* folderPath = nullptr);
	~MeshManager();

	bool Init(const char* def_shader = nullptr);
	unsigned int LoadMesh(const char* path, const bool dropped = false);
	unsigned int GetLoadedMesh(const char* path, const bool from_drop = false) const;
	void DrawMesh(const unsigned int reference);

private:

	unsigned int AddMesh(RE_MeshContainer* mesh);
	
private:

	const char* folderPath = nullptr;
	unsigned int default_shader = 0;
};

#endif // !__MESHMANAGER_H__