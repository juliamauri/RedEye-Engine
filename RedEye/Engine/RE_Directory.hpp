#ifndef __RE_DIRECTORY_HPP__
#define __RE_DIRECTORY_HPP__


#include <EASTL/string.h>
#include <EASTL/list.h>
#include <EASTL/stack.h>

namespace RE_Directory {
	enum class PathType : int { D_NULL = -1, D_FOLDER, D_FILE };
	enum class FileType : int { F_NOTSUPPORTED = -1, F_NONE, F_MESH, F_TEXTURE, F_MATERIAL, F_SKYBOX, F_PARTICLEEMISSOR, F_PARTICLERENDER, F_PREFAB, F_MODEL, F_SCENE, F_META };
	enum class PathProcessType : int { P_ADDFILE, P_DELETE, P_REIMPORT, P_ADDFOLDER, };

	struct RE_File;
	struct RE_Meta;
	struct RE_Directory;
	struct RE_Path
	{
		eastl::string path;
		PathType pType = PathType::D_NULL;

		RE_File* AsFile();
		RE_Meta* AsMeta();
		RE_Directory* AsDirectory();
	};

	struct RE_File : public RE_Path
	{
		eastl::string filename;
		FileType fType = FileType::F_NONE;
		const char* extension = nullptr;
		signed long long lastModified = 0;
		signed long long lastSize = 0;

		RE_Meta* metaResource = nullptr;

		static FileType DetectExtensionAndType(const char* _path, const char*& _extension);

		RE_Path* AsPath();
		RE_Meta* AsMeta();
	};

	struct RE_Meta : public RE_File
	{
		RE_File* fromFile = nullptr;
		const char* resource = nullptr;

		bool IsModified()const;

		RE_Path* AsPath();
		RE_File* AsFile();
	};

	struct RE_ProcessPath { PathProcessType procedure; RE_Path* toProcess; };

	struct RE_Directory : public RE_Path
	{
		eastl::string name;
		RE_Directory* parent = nullptr;
		eastl::list<RE_Path*> tree;

		void AddBeforeOf(RE_Path* toAdd, eastl::list<RE_Path*>::iterator to);
		void Delete(eastl::list<RE_Path*>::iterator del);

		void SetPath(const char* path);
		eastl::list<RE_Directory*> MountTreeFolders();
		eastl::stack<RE_ProcessPath*> CheckAndApply(eastl::vector<RE_Meta*>* metaRecentlyAdded);

		eastl::stack<RE_Path*> GetDisplayingFiles() const;

		eastl::list<const RE_Directory*> FromParentToThis() const;

		RE_Path* AsPath();
	};

	namespace Assets
	{
		void Init();
		void CleanUp();

		RE_Directory* FindDirectory(const char* pathToFind);
		RE_Path* FindPath(const char* pathToFind, RE_Directory* dir);

		eastl::list<const RE_Directory*> GetDirectories();
		const RE_Directory* GetRoot();

		void DeleteFile(const char* filepath);
	}

}

#endif // !__RE_DIRECTORY_HPP__