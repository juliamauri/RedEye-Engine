#include "RE_PathTypes.h"

#include "Globals.h"
#include "RE_ConsoleLog.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_Shader.h"

#include "PhysFS\include\physfs.h"

// ===================   RE_File : public RE_Path   ===================

FileType RE_File::DetectExtensionAndType(const char* _path, const char*& _extension)
{
	static const char* extensionsSuported[12] = { "meta", "re","refab", "pupil", "sk",  "fbx", "jpg", "dds", "png", "tga", "tiff", "bmp" };

	FileType ret = F_NOTSUPPORTED;
	eastl::string modPath(_path);
	eastl::string filename = modPath.substr(modPath.find_last_of("/") + 1);
	eastl::string extensionStr = filename.substr(filename.find_last_of(".") + 1);
	eastl::transform(extensionStr.begin(), extensionStr.end(), extensionStr.begin(), [](unsigned char c) { return eastl::CharToLower(c); });

	for (uint i = 0; i < 12; i++)
	{
		if (extensionStr.compare(extensionsSuported[i]) == 0)
		{
			_extension = extensionsSuported[i];
			switch (i) {
			case 0: ret = F_META; break;
			case 1: ret = F_SCENE; break;
			case 2: ret = F_PREFAB; break;
			case 3: ret = F_MATERIAL; break;
			case 4: ret = F_SKYBOX; break;
			case 5: ret = F_MODEL; break;
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11: ret = F_TEXTURE; break;
			}
		}
	}
	return ret;
}

// ===================   RE_Meta : public RE_File   ===================

bool RE_Meta::IsModified() const
{
	return (RE_ResourceManager::At(resource)->GetType() != Resource_Type::R_SHADER
		&& fromFile->lastModified != RE_ResourceManager::At(resource)->GetLastTimeModified());
}

// ===================   RE_Directory : public RE_Path   ===================

eastl::list<RE_Meta*> RE_Directory::recent_metas;
RE_Directory::ProcessPaths RE_Directory::assetsToProcess;

void RE_Directory::SetPath(const char* _path)
{
	path = _path;
	name = eastl::string(path.c_str(), path.size() - 1);
	name = name.substr(name.find_last_of("/") + 1);
}

eastl::list<RE_Directory*> RE_Directory::MountTreeFolders()
{
	eastl::list<RE_Directory*> ret;
	if (PHYSFS_exists(path.c_str()) != 0)
	{
		eastl::string iterPath(path);
		char** rc = PHYSFS_enumerateFiles(iterPath.c_str());
		for (char** i = rc; *i != NULL; i++)
		{
			eastl::string inPath(iterPath);
			inPath += *i;

			PHYSFS_Stat fileStat;
			if (PHYSFS_stat(inPath.c_str(), &fileStat) && fileStat.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY)
			{
				RE_Directory* newDirectory = new RE_Directory();
				newDirectory->SetPath((inPath += "/").c_str());
				newDirectory->parent = this;
				newDirectory->pType = D_FOLDER;
				tree.push_back(static_cast<RE_Path*>(newDirectory));
				ret.push_back(newDirectory);
			}
		}
		PHYSFS_freeList(rc);

		if (!tree.empty())
		{
			for (RE_Path* path : tree)
			{
				eastl::list< RE_Directory*> fromChild = static_cast<RE_Directory*>(path)->MountTreeFolders();
				if (!fromChild.empty()) ret.insert(ret.end(), fromChild.begin(), fromChild.end());
			}
		}
	}
	return ret;
}

eastl::stack<RE_Path*> RE_Directory::GetDisplayingFiles() const
{
	eastl::stack<RE_Path*> ret;

	for (auto path : tree)
	{
		switch (path->pType) {
		case PathType::D_FILE:
		{
			switch (static_cast<RE_File*>(path)->fType)
			{
			case FileType::F_NOTSUPPORTED: break;
			case FileType::F_NONE: break;
			case FileType::F_META:
			{
				const char* res = static_cast<RE_Meta*>(path)->resource;
				if (res != nullptr && RE_ResourceManager::At(res)->GetType() == R_SHADER) ret.push(path);
				break;
			}
			default: ret.push(path); break; }

			break;
		}
		case PathType::D_FOLDER: ret.push(path); break;
		default: break; }
	}

	return ret;
}

eastl::list<RE_Directory*> RE_Directory::FromParentToThis()
{
	eastl::list<RE_Directory*> ret;
	RE_Directory* iter = this;
	while (iter != nullptr)
	{
		ret.push_front(iter);
		iter = iter->parent;
	}
	return ret;
}

void RE_Directory::CheckAndApply()
{
	if (PHYSFS_exists(path.c_str()) != 0)
	{
		eastl::string iterPath(path);
		eastl::vector<RE_Meta*> toRemoveM;
		char** rc = PHYSFS_enumerateFiles(iterPath.c_str());
		auto iter = tree.begin();
		for (char** i = rc; *i != NULL; i++)
		{
			PathType iterTreeType = (iter != tree.end()) ? (*iter)->pType : PathType::D_NULL;
			eastl::string inPath(iterPath);
			inPath += *i;
			PHYSFS_Stat fileStat;
			if (PHYSFS_stat(inPath.c_str(), &fileStat))
			{
				if (fileStat.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY)
				{
					bool newFolder = (iterTreeType == PathType::D_NULL || iterTreeType != PathType::D_FOLDER || (*iter)->path != (inPath += "/"));
					if (newFolder && iter != tree.end())
					{
						auto postPath = iter;
						bool found = false;
						while (postPath != tree.end())
						{
							if (inPath == (*postPath)->path)
							{
								found = true;
								break;
							}
							postPath++;
						}
						if (found) newFolder = false;
					}

					if (newFolder)
					{
						RE_Directory* newDirectory = new RE_Directory();

						if (inPath.back() != '/')inPath += "/";
						newDirectory->SetPath(inPath.c_str());
						newDirectory->parent = this;
						newDirectory->pType = D_FOLDER;

						AddBeforeOf(newDirectory, iter);
						iter--;

						assetsToProcess.push_back({ P_ADDFOLDER, static_cast<RE_Path*>(newDirectory) });
					}
				}
				else if (fileStat.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_REGULAR)
				{
					const char* extension = nullptr;
					FileType fileType = RE_File::DetectExtensionAndType(inPath.c_str(), extension);
					bool newFile = (iterTreeType == PathType::D_NULL || iterTreeType != PathType::D_FILE || (*iter)->path != inPath);
					if (newFile && fileType == FileType::F_META && !recent_metas.empty())
					{
						int sizemetap = 0;
						for (RE_Meta* metaAdded : recent_metas)
						{
							sizemetap = eastl::CharStrlen(metaAdded->path.c_str());
							if (sizemetap > 0 && eastl::Compare(metaAdded->path.c_str(), inPath.c_str(), sizemetap) == 0)
							{
								AddBeforeOf(static_cast<RE_Path*>(metaAdded), iter);
								iter--;
								toRemoveM.push_back(metaAdded);
								recent_metas.erase(eastl::remove_if(begin(recent_metas), end(recent_metas),
									[&](auto x) {return eastl::find(begin(toRemoveM), end(toRemoveM), x) != end(toRemoveM); }), eastl::end(recent_metas));
								toRemoveM.clear();
								newFile = false;
								break;
							}
						}
					}

					if (newFile && iter != tree.end())
					{
						auto postPath = iter;
						bool found = false;
						while (postPath != tree.end())
						{
							if (inPath == (*postPath)->path)
							{
								found = true;
								break;
							}
							postPath++;
						}
						if (found) newFile = false;
					}

					if (newFile)
					{
						RE_File* newFile = (fileType != FileType::F_META) ? new RE_File() : static_cast<RE_File*>(new RE_Meta());
						newFile->path = inPath;
						newFile->filename = inPath.substr(inPath.find_last_of("/") + 1);
						newFile->lastSize = fileStat.filesize;
						newFile->pType = PathType::D_FILE;
						newFile->fType = fileType;
						newFile->extension = extension;

						RE_Path* path = static_cast<RE_Path*>(newFile);
						assetsToProcess.push_back({ P_ADDFILE, path });

						RE_LOG("SIZE = %i", assetsToProcess.size());

						AddBeforeOf(path, iter);
						iter--;
					}
					else if (static_cast<RE_File*>(*iter)->fType == FileType::F_META)
					{
						bool reimport = false;
						ResourceContainer* res = RE_ResourceManager::At(static_cast<RE_Meta*>(*iter)->resource);
						if (res->GetType() == Resource_Type::R_SHADER && static_cast<RE_Shader*>(res)->isShaderFilesChanged())
							assetsToProcess.push_back({ P_REIMPORT, (*iter) });
					}
				}
			}

			if (iter != tree.end()) iter++;
		}

		PHYSFS_freeList(rc);
	}
}
