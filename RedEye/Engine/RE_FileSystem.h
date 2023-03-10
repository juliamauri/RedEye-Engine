#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <EASTL\list.h>
#include <EASTL\vector.h>
#include <EASTL\stack.h>
#include <EASTL\string.h>
#include <EASTL\priority_queue.h>

class Config;
class RE_Json;
class RE_FileBuffer;
class RE_GameObject;
class ResourceContainer;
struct Vertex;

class RE_FileSystem
{
public:
	RE_FileSystem() {}
	~RE_FileSystem() {}

	bool Init(int argc, char* argv[]);
	void Clear();

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

	void DeleteUndefinedFile(const char* filePath);
	void DeleteResourceFiles(ResourceContainer* resContainer);

	signed long long GetLastTimeModified(const char* path);

	RE_Json* ConfigNode(const char* node) const;
	void SaveConfig() const;

private:

	void CopyDirectory(const char* origin, const char* dest);

private:

	eastl::string engine_path, project_path, library_path, assets_path, write_path, pref_directory;

	Config* config = nullptr;
};

#endif // !__FILESYSTEM_H__