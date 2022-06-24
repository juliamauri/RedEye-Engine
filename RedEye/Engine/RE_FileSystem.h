#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <EASTL\list.h>
#include <EASTL\vector.h>
#include <EASTL\stack.h>
#include <EASTL\string.h>

class Config;
class RE_Json;
class RE_FileBuffer;
class RE_GameObject;
class ResourceContainer;
struct Vertex;

class RE_FileSystem
{
public:

	enum PathType { D_NULL = -1, D_FOLDER, D_FILE };
	enum FileType { F_NOTSUPPORTED = -1, F_NONE, F_MODEL, F_TEXTURE, F_MATERIAL, F_SKYBOX, F_PREFAB, F_SCENE, F_PARTICLEEMISSOR, F_PARTICLERENDER, F_META };
	enum PathProcessType { P_ADDFILE, P_DELETE, P_REIMPORT, P_ADDFOLDER, };

	struct RE_File;
	struct RE_Meta;
	struct RE_Directory;
	struct RE_Path
	{
		eastl::string path;
		PathType pType = D_NULL;

		RE_File* AsFile()const { return (RE_File*)this; }
		RE_Meta* AsMeta()const { return (RE_Meta*)this; }
		RE_Directory* AsDirectory()const { return (RE_Directory*)this; }
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

		RE_Path* AsPath()const { return (RE_Path*)this; }
		RE_Meta* AsMeta()const { return (RE_Meta*)this; }
	};

	struct RE_Meta : public RE_File
	{
		RE_File* fromFile = nullptr;
		const char* resource = nullptr;
		
		bool IsModified()const;

		RE_Path* AsPath()const { return (RE_Path*)this; }
		RE_File* AsFile()const { return (RE_File*)this; }
	};

	struct RE_ProcessPath { PathProcessType procedure; RE_Path* toProcess; };

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

	const char* GetZipPath();

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

	eastl::string engine_path, library_path, assets_path, zip_path, write_path;

	RE_Directory* rootAssetDirectory = nullptr;
	eastl::list<RE_Directory*> assetsDirectories;
	eastl::list<RE_Directory*>::iterator dirIter;
	eastl::stack<RE_ProcessPath*> assets_to_process;
	eastl::stack<RE_ProcessPath*> meta_to_process_last;

	eastl::vector<RE_Meta*> metaToFindFile;
	eastl::stack<RE_Meta*> reloadResourceMeta;
	eastl::vector<RE_File*> filesToFindMeta;
	eastl::vector<RE_Meta*> metaRecentlyAdded;

	eastl::list<RE_File*> toImport;
	eastl::list<RE_Meta*> toReImport;

	Config* config = nullptr;
};

#endif // !__FILESYSTEM_H__