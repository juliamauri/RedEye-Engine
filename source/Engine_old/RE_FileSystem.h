#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "RE_DataTypes.h"
#include <EASTL/list.h>
#include <EASTL/vector.h>
#include <EASTL/stack.h>
#include <EASTL/string.h>
#include <EASTL/priority_queue.h>

class Config;
class RE_Json;
class RE_FileBuffer;
class RE_GameObject;
class ResourceContainer;
struct Vertex;

class RE_FileSystem
{
public:

	enum class PathType : short
	{
		NONE = -1,
		FOLDER,
		FILE
	};

	enum class FileType : short
	{
		NOTSUPPORTED = -1,
		NONE,
		MESH,
		TEXTURE,
		MATERIAL,
		SKYBOX,
		PARTICLEEMISSOR,
		PARTICLERENDER,
		PREFAB,
		MODEL,
		SCENE,
		META
	};

	enum class PathProcess : ushort
	{
		ADDFILE,
		DELETE,
		REIMPORT,
		ADDFOLDER,
	};

	struct RE_File;
	struct RE_Meta;
	struct RE_Directory;
	struct RE_Path
	{
		eastl::string path;
		PathType pType = PathType::NONE;

		RE_File* AsFile() const { return (RE_File*)this; }
		RE_Meta* AsMeta() const { return (RE_Meta*)this; }
		RE_Directory* AsDirectory() const { return (RE_Directory*)this; }
	};

	struct RE_File : public RE_Path
	{
		eastl::string filename;
		FileType fType = FileType::NONE;
		const char* extension = nullptr;
		signed long long lastModified = 0;
		signed long long lastSize = 0;

		RE_Meta* metaResource = nullptr;

		static FileType DetectExtensionAndType(const char* _path, const char*& _extension);

		RE_Path* AsPath() const { return (RE_Path*)this; }
		RE_Meta* AsMeta() const { return (RE_Meta*)this; }
	};

	struct RE_Meta : public RE_File
	{
		RE_File* fromFile = nullptr;
		const char* resource = nullptr;
		
		bool IsModified() const;

		RE_Path* AsPath() const { return (RE_Path*)this; }
		RE_File* AsFile() const { return (RE_File*)this; }
	};

	struct RE_ProcessPath
	{
		PathProcess procedure;
		RE_Path* toProcess;

		RE_ProcessPath(const PathProcess& procedure, RE_Path* toProcess)
			: procedure(procedure), toProcess(toProcess)
		{}
	};

	struct RE_Directory : public RE_Path
	{
		eastl::string name;
		RE_Directory* parent = nullptr;
		eastl::list<RE_Path*> tree;

		void AddBeforeOf(RE_Path* toAdd, eastl::list<RE_Path*>::iterator to) { tree.insert(to, toAdd); }
		void Delete(eastl::list<RE_Path*>::iterator del) { tree.erase(del); }

		void SetPath(const char* path);
		eastl::list<RE_Directory*> MountTreeFolders();
		eastl::stack<RE_ProcessPath*> CheckAndApply(eastl::vector<RE_Meta*>* metaRecentlyAdded);

		eastl::stack<RE_Path*> GetDisplayingFiles() const;

		eastl::list<RE_Directory*> FromParentToThis();

		RE_Path* AsPath()const { return (RE_Path*)this; }
	};

public:

	RE_FileSystem() {}
	~RE_FileSystem() {}

	bool Init(int argc, char* argv[]);
	void Clear();

	unsigned int ReadAssetChanges(unsigned int extra_ms, bool doAll = false);

	void DrawEditor();
	bool AddPath(const char* path_or_zip, const char* mount_point = nullptr);
	bool RemovePath(const char* path_or_zip);
	bool SetWritePath(const char* dir);
	const char* GetWritePath() const;
	void LogFolderItems(const char* folder);

	RE_FileBuffer* QuickBufferFromPDPath(const char* full_path); // , char** buffer, unsigned int size);

	bool ExistsOnOSFileSystem(const char* file, bool isFolder = true) const;
	bool Exists(const char* file) const;
	bool IsDirectory(const char* file) const;
	const char* GetExecutableDirectory() const;

	void HandleDropedFile(const char* file);

	RE_Directory* GetRootDirectory()const;

	void DeleteUndefinedFile(const char* filePath);
	void DeleteResourceFiles(ResourceContainer* resContainer);

	RE_Directory* FindDirectory(const char* pathToFind);
	RE_Path* FindPath(const char* pathToFind, RE_Directory* dir = nullptr);

	signed long long GetLastTimeModified(const char* path);

	RE_Json* ConfigNode(const char* node) const;
	void SaveConfig() const;

private:

	void CopyDirectory(const char* origin, const char* dest);

private:

	eastl::string engine_path, project_path, library_path, assets_path, write_path, pref_directory;

	RE_Directory* rootAssetDirectory = nullptr;
	eastl::list<RE_Directory*> assetsDirectories;
	eastl::list<RE_Directory*>::iterator dirIter;
	eastl::stack<RE_ProcessPath*> assets_to_process;
	eastl::stack<RE_ProcessPath*> meta_to_process_last;

	eastl::vector<RE_Meta*> metaToFindFile;
	eastl::stack<RE_Meta*> reloadResourceMeta;
	eastl::vector<RE_File*> filesToFindMeta;
	eastl::vector<RE_Meta*> metaRecentlyAdded;

	struct AssetsPrioroty
	{
		bool operator()(const RE_File* lhs, const RE_File* rhs) const
		{
			return lhs->fType > rhs->fType;
		}
	};

	eastl::priority_queue<RE_File*, eastl::vector<RE_File*>, AssetsPrioroty> toImport;
	eastl::priority_queue<RE_File*, eastl::vector<RE_File*>, AssetsPrioroty> toReImport;

	Config* config = nullptr;
};

#endif // !__FILESYSTEM_H__