#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <EASTL\stack.h>
#include <EASTL\list.h>
#include <EASTL\vector.h>
#include <EASTL\string.h>

struct RE_Directory;
struct RE_Path;
struct RE_File;
struct RE_Meta;
class Config;
class RE_Json;
class RE_FileBuffer;
class ResourceContainer;

namespace RE_FileSystem
{
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

	// Config
	RE_Json* GetConfigNode(const char* node);
	void SaveConfig();

	namespace Internal
	{
		void CopyDirectory(const char* origin, const char* dest);

		static Config* config = nullptr;
		static RE_Directory* rootAssetDirectory = nullptr;

		typedef eastl::list<RE_Directory*> DirectoryPool;
		static DirectoryPool assetsDirectories;
		static DirectoryPool::iterator dirIter;

		typedef eastl::vector<RE_Meta*> MetaPool;
		static MetaPool metaToFindFile;
		static MetaPool metaRecentlyAdded;

		static eastl::vector<RE_File*> filesToFindMeta;
		static eastl::list<RE_File*> toImport;

		static eastl::list<RE_Meta*> meta_reimports;

		static const char* engine_path = "engine";
		static const char* library_path = "Library";
		static const char* assets_path = "Assets";

		static eastl::string zip_path;
		static eastl::string write_path;
	}
};

#endif // !__FILESYSTEM_H__