#pragma once

#include <EASTL\stack.h>
#include <EASTL\list.h>
#include "EASTL/utility.h"
#include <EASTL\string.h>

struct RE_File;
struct RE_Meta;
struct RE_Directory;

enum PathType { D_NULL = -1, D_FOLDER, D_FILE };
enum FileType { F_NOTSUPPORTED = -1, F_NONE, F_MODEL, F_TEXTURE, F_MATERIAL, F_SKYBOX, F_PREFAB, F_SCENE, F_META };
enum PathProcessType { P_ADDFILE, P_DELETE, P_REIMPORT, P_ADDFOLDER, };

struct RE_Path
{
	eastl::string path;
	PathType pType = D_NULL;
};

struct RE_File : public RE_Path
{
	eastl::string filename;
	FileType fType = F_NONE;
	const char* extension = nullptr;
	signed long long lastModified = 0;
	signed long long lastSize = 0;

	RE_Meta* metaResource = nullptr;

	static FileType DetectExtensionAndType(const char* _path, const char*& _extension);
};

struct RE_Meta : public RE_File
{
	RE_File* fromFile = nullptr;
	const char* resource = nullptr;

	bool IsModified()const;
};

struct PathProcess { int type; RE_Path* path; };

struct RE_Directory : public RE_Path
{
	eastl::string name;
	RE_Directory* parent = nullptr;
	eastl::list<RE_Path*> tree;

	void AddBeforeOf(RE_Path* toAdd, eastl::list<RE_Path*>::iterator to) { tree.insert(to, toAdd); }
	void Delete(eastl::list<RE_Path*>::iterator del) { tree.erase(del); }

	void SetPath(const char* path);
	eastl::list<RE_Directory*> MountTreeFolders();
	eastl::stack<RE_Path*> GetDisplayingFiles() const;
	eastl::list<RE_Directory*> FromParentToThis();

	void CheckAndApply();

	static eastl::list<RE_Meta*> recent_metas;

	typedef eastl::vector<PathProcess> ProcessPaths;
	static ProcessPaths assetsToProcess;
};
