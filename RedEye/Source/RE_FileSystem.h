#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <EASTL\list.h>
#include <EASTL\vector.h>
#include <EASTL\stack.h>
#include <EASTL\string.h>

class Config;
class RE_FileBuffer;
class ResourceContainer;

namespace RE_FileSystem
{
	enum PathType { D_NULL = -1, D_FOLDER, D_FILE };
	enum FileType { F_NOTSUPPORTED = -1, F_NONE, F_MODEL, F_TEXTURE, F_MATERIAL, F_SKYBOX, F_PREFAB, F_SCENE, F_META };
	enum PathProcessType { P_ADDFILE, P_DELETE, P_REIMPORT, P_ADDFOLDER, };

	struct RE_ProcessPath;
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

		eastl::stack<RE_Path*> GetDisplayingFiles()const;

		eastl::list<RE_Directory*> FromParentToThis();

		RE_Path* AsPath()const { return (RE_Path*)this; }
	};

	static Config* config = nullptr;

	bool Init(int argc, char* argv[]);
	void Clear();

	unsigned int ReadAssetChanges(unsigned int extra_ms, bool doAll = false);

	void DrawEditor();
	bool AddPath(const char* path_or_zip, const char* mount_point = nullptr);
	bool RemovePath(const char* path_or_zip);
	bool SetWritePath(const char* dir);
	const char* GetWritePath();
	void LogFolderItems(const char* folder);

	RE_FileBuffer* QuickBufferFromPDPath(const char* full_path); // , char** buffer, unsigned int size);

	bool ExistsOnOSFileSystem(const char* file, bool isFolder = true);
	bool Exists(const char* file);
	bool IsDirectory(const char* file);
	const char* GetExecutableDirectory();

	const char* GetZipPath();

	void HandleDropedFile(const char* file);

	RE_Directory* GetRootDirectory();

	void DeleteUndefinedFile(const char* filePath);
	void DeleteResourceFiles(ResourceContainer* resContainer);

	RE_Directory* FindDirectory(const char* pathToFind);
	RE_Path* FindPath(const char* pathToFind, RE_Directory* dir = nullptr);

	signed long long GetLastTimeModified(const char* path);

	namespace Internal
	{
		void CopyDirectory(const char* origin, const char* dest);

		static eastl::string engine_path;
		static eastl::string library_path;
		static eastl::string assets_path;
		static eastl::string zip_path;
		static eastl::string write_path;

		static RE_Directory* rootAssetDirectory = nullptr;
		static eastl::list<RE_Directory*> assetsDirectories;
		static eastl::list<RE_Directory*>::iterator dirIter;
		static eastl::stack<RE_ProcessPath*> assetsToProcess;

		static eastl::vector<RE_Meta*> metaToFindFile;
		static eastl::vector<RE_File*> filesToFindMeta;
		static eastl::vector<RE_Meta*> metaRecentlyAdded;

		static eastl::list<RE_File*> toImport;
		static eastl::list<RE_Meta*> toReImport;
	}
};

#endif // !__FILESYSTEM_H__