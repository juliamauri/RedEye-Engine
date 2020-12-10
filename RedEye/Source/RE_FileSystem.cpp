#include "RE_FileSystem.h"

#include "RE_Time.h"
#include "RE_ConsoleLog.h"
#include "RE_PathTypes.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "RE_ResourceManager.h"

#include "ImGui\imgui.h"
#include "libzip\include\zip.h"
#include "PhysFS\include\physfs.h"
#include <EASTL\internal\char_traits.h>
#include <EASTL\algorithm.h>
#include <EASTL\iterator.h>
#include <EAStdC\EASprintf.h>

#pragma comment( lib, "PhysFS/libx86/physfs.lib" )

#ifdef _DEBUG
	#pragma comment( lib, "libzip/zip_d.lib" )
#else
	#pragma comment( lib, "libzip/zip_r.lib" )
#endif // _DEBUG

using namespace RE_FileSystem::Internal;

bool RE_FileSystem::Init(int argc, char* argv[])
{
	bool ret = false;
	RE_LOG("Initializing File System");
	if (PHYSFS_init(argv[0]) != 0)
	{
		PHYSFS_Version physfs_version;
		PHYSFS_VERSION(&physfs_version);
		char tmp[8];
		EA::StdC::Snprintf(tmp, 8, "%u.%u.%u", static_cast<int>(physfs_version.major), static_cast<int>(physfs_version.minor), static_cast<int>(physfs_version.patch));
		RE_SOFT_NVS("PhysFS", tmp, "https://icculus.org/physfs/");
		RE_SOFT_NVS("Rapidjson", RAPIDJSON_VERSION_STRING, "http://rapidjson.org/");
		RE_SOFT_NVS("LibZip", "1.5.0", "https://libzip.org/");

		zip_path = GetExecutableDirectory();
		if (PHYSFS_mount((zip_path += "data.zip").c_str(), NULL, 1) != 0)
		{
			char** i;
			for (i = PHYSFS_getSearchPath(); *i != NULL; i++) RE_LOG("[%s] is in the search path.\n", *i);
			PHYSFS_freeList(*i);

			config = new Config("Settings/config.json", zip_path.c_str());
			if (ret = config->Load())
			{
				rootAssetDirectory = new RE_Directory();
				rootAssetDirectory->SetPath("Assets/");
				assetsDirectories = rootAssetDirectory->MountTreeFolders();
				assetsDirectories.push_front(rootAssetDirectory);
				dirIter = assetsDirectories.begin();
			}
			else RE_LOG_WARNING("Can't load Settings/config.json - building module default configuration.");
		}
		else RE_LOG_ERROR("PhysFS failed to mount data.zip(%s) Error: %s", zip_path.c_str(), PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	}
	else RE_LOG_ERROR("PhysFS could not initialize! Error: %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

	return ret;
}

void RE_FileSystem::Clear()
{
	for (auto dir : assetsDirectories)
		for (auto p : dir->tree)
			if (p->pType != PathType::D_FOLDER)
				DEL(p);

	for (auto dir : assetsDirectories) DEL(dir);

	DEL(config);

	PHYSFS_deinit();
}

unsigned int RE_FileSystem::ReadAssetChanges(unsigned int extra_ms, bool doAll)
{
	OPTICK_CATEGORY("Read Asset Changes - File system", Optick::Category::IO);

	Timer time;
	bool run = true;

	if ((dirIter == assetsDirectories.begin()) && (
		!RE_Directory::assetsToProcess.empty() ||
		!filesToFindMeta.empty() ||
		!toImport.empty() ||
		!meta_reimports.empty())) dirIter = assetsDirectories.end();

	while ((doAll || run) && dirIter != assetsDirectories.end())
	{
		(*dirIter)->CheckAndApply();
		dirIter++;

		//timer
		if (!doAll && extra_ms < time.Read()) run = false;
	}

	if (dirIter == assetsDirectories.end())
	{
		dirIter = assetsDirectories.begin();

		while ((doAll || run) && !RE_Directory::assetsToProcess.empty())
		{
			PathProcess process = RE_Directory::assetsToProcess.back();
			switch (static_cast<PathProcessType>(process.type))
			{
			case P_ADDFILE:
			{
				RE_File* file = static_cast<RE_File*>(process.path);
				switch (file->fType)
				{
				case F_NOTSUPPORTED: RE_LOG_ERROR("Unsupported file type"); break;
				case F_NONE: RE_LOG_ERROR("Missing file type"); break;
				case F_META:
				{
					const char* isReferenced = RE_ResourceManager::FindMD5ByMETAPath(file->path.c_str());

					RE_Meta* metaFile = static_cast<RE_Meta*>(file);
					if (!isReferenced)
					{
						Config metaLoad(file->path.c_str(), GetZipPath());
						if (metaLoad.Load())
						{
							RE_Json* metaNode = metaLoad.GetRootNode("meta");
							Resource_Type type = (Resource_Type)metaNode->PullInt("Type", Resource_Type::R_UNDEFINED);
							DEL(metaNode);

							if (type != Resource_Type::R_UNDEFINED)
								metaFile->resource = RE_ResourceManager::ReferenceByMeta(file->path.c_str(), type);
						}
					}
					else metaFile->resource = isReferenced;

					metaToFindFile.push_back(metaFile);
					break;
				}
				default: filesToFindMeta.push_back(file); break;
				}
				break;
			}
			case P_ADDFOLDER:
			{
				RE_Directory* dir = static_cast<RE_Directory*>(process.path);
				dir->CheckAndApply();
				assetsDirectories.push_back(dir);
				break;
			}
			case P_REIMPORT:
			{
				meta_reimports.push_back(static_cast<RE_Meta*>(process.path));
				break;
			}
			}

			RE_Directory::assetsToProcess.pop_back();
			if (!doAll && extra_ms < time.Read()) run = false;
		}

		if ((doAll || run) && !metaToFindFile.empty())
		{
			eastl::vector<RE_File*> toRemoveF;
			eastl::vector<RE_Meta*> toRemoveM;

			for (RE_Meta* meta : metaToFindFile)
			{
				ResourceContainer* res = RE_ResourceManager::At(meta->resource);

				if (!res->isInternal())
				{
					int sizefile = 0;
					if (!filesToFindMeta.empty())
					{
						for (RE_File* file : filesToFindMeta)
						{
							const char* assetPath = res->GetAssetPath();
							sizefile = eastl::CharStrlen(assetPath);
							if (sizefile > 0 && eastl::Compare(assetPath, file->path.c_str(), sizefile) == 0)
							{
								meta->fromFile = file;
								file->metaResource = meta;
								toRemoveF.push_back(file);
								toRemoveM.push_back(meta);
							}
						}
					}
					if (res->GetType() == R_SHADER)  toRemoveM.push_back(meta);
				}
				else toRemoveM.push_back(meta);

				//timer
				if (!doAll && extra_ms < time.Read()) run = false;
			}
			if (!toRemoveF.empty())
			{
				//https://stackoverflow.com/questions/21195217/elegant-way-to-remove-all-elements-of-a-vector-that-are-contained-in-another-vec
				filesToFindMeta.erase(eastl::remove_if(eastl::begin(filesToFindMeta), eastl::end(filesToFindMeta),
					[&](auto x) {return eastl::find(begin(toRemoveF), end(toRemoveF), x) != end(toRemoveF); }), eastl::end(filesToFindMeta));
			}
			if (!toRemoveM.empty())
			{
				metaToFindFile.erase(eastl::remove_if(eastl::begin(metaToFindFile), eastl::end(metaToFindFile),
					[&](auto x) {return eastl::find(begin(toRemoveM), end(toRemoveM), x) != end(toRemoveM); }), eastl::end(metaToFindFile));
			}
		}

		if ((doAll || run) && !filesToFindMeta.empty())
		{
			for (RE_File* file : filesToFindMeta)
			{
				switch (file->fType)
				{
				case F_MODEL:
				case F_PREFAB:
				case F_SCENE:
					toImport.push_back(file);
					break;
				case F_SKYBOX:
				{
					eastl::list<RE_File*>::const_iterator it = toImport.end();
					if (!toImport.empty())
					{
						it--;
						FileType type = (*it)->fType;
						int count = toImport.size();
						while (count > 1)
						{
							if (type == F_PREFAB || type == F_SCENE || type == F_MODEL) break;

							count--;
							type = (*(--it))->fType;
						}
						it++;
					}
					toImport.insert(it, file);
					break;
				}
				case F_TEXTURE:
				case F_MATERIAL:
					toImport.push_front(file);
					break;
				}
			}
			filesToFindMeta.clear();
		}

		if ((doAll || run) && !toImport.empty())
		{
			RE_ConsoleLog::ScopeProcedureLogging();
			eastl::vector<RE_File*> toRemoveF;

			//Importing
			for (RE_File* file : toImport) {
				RE_LOG("Importing %s", file->path.c_str());

				const char* newRes = nullptr;
				switch (file->fType) {
				case F_MODEL:	 newRes = RE_ResourceManager::ImportModel(file->path.c_str()); break;
				case F_TEXTURE:	 newRes = RE_ResourceManager::ImportTexture(file->path.c_str()); break;
				case F_MATERIAL: newRes = RE_ResourceManager::ImportMaterial(file->path.c_str()); break;
				case F_SKYBOX:	 newRes = RE_ResourceManager::ImportSkyBox(file->path.c_str()); break;
				case F_PREFAB:	 newRes = RE_ResourceManager::ImportPrefab(file->path.c_str()); break;
				case F_SCENE:	 newRes = RE_ResourceManager::ImportScene(file->path.c_str()); break; }

				if (newRes != nullptr)
				{
					RE_Meta* newMetaFile = new RE_Meta();
					newMetaFile->resource = newRes;
					newMetaFile->fromFile = file;
					newMetaFile->fType = FileType::F_META;
					newMetaFile->path = RE_ResourceManager::At(newRes)->GetMetaPath();
					newMetaFile->pType = PathType::D_FILE;
					RE_Directory::recent_metas.push_back(file->metaResource = newMetaFile);
				}
				else
					file->fType = F_NOTSUPPORTED;

				toRemoveF.push_back(file);

				if (!doAll && extra_ms < time.Read())
				{
					run = false;
					break;
				}
			}

			if (!toRemoveF.empty())
			{
				//https://stackoverflow.com/questions/21195217/elegant-way-to-remove-all-elements-of-a-vector-that-are-contained-in-another-vec
				toImport.erase(eastl::remove_if(eastl::begin(toImport), eastl::end(toImport),
					[&](auto x) {return eastl::find(begin(toRemoveF), end(toRemoveF), x) != end(toRemoveF); }), eastl::end(toImport));
			}

			RE_ConsoleLog::EndScope();
		}

		if ((doAll || run) && !meta_reimports.empty())
		{
			eastl::vector<RE_Meta*> toRemoveM;
			for (RE_Meta* meta : meta_reimports)
			{
				RE_LOG("ReImporting %s", meta->path.c_str());
				RE_ResourceManager::At(meta->resource)->ReImport();
				toRemoveM.push_back(meta);

				if (!doAll && extra_ms < time.Read())
				{
					run = false;
					break;
				}
			}

			if (!toRemoveM.empty())
			{
				//https://stackoverflow.com/questions/21195217/elegant-way-to-remove-all-elements-of-a-vector-that-are-contained-in-another-vec
				meta_reimports.erase(eastl::remove_if(eastl::begin(meta_reimports), eastl::end(meta_reimports),
					[&](auto x) {return eastl::find(begin(toRemoveM), end(toRemoveM), x) != end(toRemoveM); }), eastl::end(meta_reimports));
			}
		}
	}

	unsigned int realTime = time.Read();
	return (extra_ms < realTime) ? 0 : extra_ms - realTime;
}

void RE_FileSystem::DrawEditor()
{
	ImGui::Text("Executable Directory:");
	ImGui::TextWrappedV(GetExecutableDirectory(), "");

	ImGui::Separator();
	ImGui::Text("All assets directories:");
	for (auto dir : assetsDirectories)ImGui::Text(dir->path.c_str());
	ImGui::Separator();

	ImGui::Text("Write Directory");
	ImGui::TextWrappedV(write_path.c_str(), "");
}

bool RE_FileSystem::AddPath(const char * path_or_zip, const char * mount_point)
{
	bool ret = true;

	if (PHYSFS_mount(path_or_zip, mount_point, 1) == 0)
	{
		RE_LOG_ERROR("File System error while adding a path or zip(%s): %s\n", path_or_zip, PHYSFS_getLastError());
		ret = false;
	}
	return ret;
}

bool RE_FileSystem::RemovePath(const char * path_or_zip)
{
	bool ret = true;

	if (PHYSFS_removeFromSearchPath(path_or_zip) == 0)
	{
		RE_LOG_ERROR("Error removing PhysFS Directory (%s): %s", path_or_zip, PHYSFS_getLastError());
		ret = false;
	}
	return ret;
}

bool RE_FileSystem::SetWritePath(const char * dir)
{
	bool ret = true;

	if (!PHYSFS_setWriteDir(dir))
	{
		RE_LOG_ERROR("Error setting PhysFS Directory: %s", PHYSFS_getLastError());
		ret = false;
	}
	else write_path = dir;

	return ret;
}

const char * RE_FileSystem::GetWritePath() { return write_path.c_str(); }

void RE_FileSystem::LogFolderItems(const char * folder)
{
	char **rc = PHYSFS_enumerateFiles(folder), **i;
	for (i = rc; *i != NULL; i++) RE_LOG(" * We've got [%s].\n", *i);
	PHYSFS_freeList(rc);
}

// Quick Buffer From Platform-Dependent Path
RE_FileBuffer* RE_FileSystem::QuickBufferFromPDPath(const char * full_path)// , char** buffer, unsigned int size)
{
	RE_FileBuffer* ret = nullptr;
	if (full_path)
	{
		eastl::string file_path = full_path;
		eastl::string file_name = file_path.substr(file_path.find_last_of("\\") + 1);
		eastl::string ext = file_name.substr(file_name.find_last_of(".") + 1);
		file_path.erase(file_path.length() - file_name.length(), file_path.length());

		eastl::string tmp = "/Export/";
		tmp += file_name;
		ret = new RE_FileBuffer(tmp.c_str());

		if (AddPath(file_path.c_str(), "/Export/"))
		{
			if (!ret->Load()) DEL(ret);
			RemovePath(file_path.c_str());
		}
		else DEL(ret);
	}

	return ret;
}

bool RE_FileSystem::ExistsOnOSFileSystem(const char* path, bool isFolder)
{
	eastl::string full_path(path);
	eastl::string directory = full_path.substr(0, full_path.find_last_of('\\') + 1);
	eastl::string fileNameExtension = full_path.substr(full_path.find_last_of("\\") + 1);

	eastl::string tempPath("/Check/");
	if (PHYSFS_mount(directory.c_str(), "/Check/", 1) != 0)
	{
		if (!isFolder)
		{
			if (Exists((tempPath += fileNameExtension).c_str()))
			{
				PHYSFS_removeFromSearchPath(directory.c_str());
				return true;
			}
		}
		PHYSFS_removeFromSearchPath(directory.c_str());
		return true;
	}
	return false;
}

bool RE_FileSystem::Exists(const char* file) { return PHYSFS_exists(file) != 0; }
bool RE_FileSystem::IsDirectory(const char* file) { return PHYSFS_isDirectory(file) != 0; }
const char* RE_FileSystem::GetExecutableDirectory() { return PHYSFS_getBaseDir(); }
const char * RE_FileSystem::GetZipPath() { return zip_path.c_str(); }

void RE_FileSystem::HandleDropedFile(const char * file)
{
	OPTICK_CATEGORY("Dropped File", Optick::Category::IO);
	eastl::string full_path(file);
	eastl::string directory = full_path.substr(0, full_path.find_last_of('\\') + 1);
	eastl::string fileNameExtension = full_path.substr(full_path.find_last_of("\\") + 1);
	eastl::string fileName = fileNameExtension.substr(0, fileNameExtension.find_last_of("."));
	eastl::string ext = full_path.substr(full_path.find_last_of(".") + 1);
	eastl::string exportPath("Exporting/");
	eastl::string finalPath = exportPath;

	bool newDir = (ext.compare("zip") == 0 || ext.compare("7z") == 0 || ext.compare("iso") == 0);
	if (!newDir)
	{
		RE_FileBuffer* fileLoaded = QuickBufferFromPDPath(file);
		if (fileLoaded)
		{
			eastl::string destPath = App::editor->GetAssetsPanelPath();
			destPath += fileNameExtension;
			RE_FileBuffer toSave(destPath.c_str(), GetZipPath());
			toSave.Save(fileLoaded->GetBuffer(), fileLoaded->GetSize());
		}
		else newDir = true;
	}

	if (newDir)
	{
		finalPath += fileName + "/";
		AddPath(file, finalPath.c_str());
		CopyDirectory(exportPath.c_str(), App::editor->GetAssetsPanelPath());
		RemovePath(file);
	}
}

RE_Directory* RE_FileSystem::GetRootDirectory() { return rootAssetDirectory; }

void RE_FileSystem::DeleteUndefinedFile(const char* filePath)
{
	RE_Directory* dir = FindDirectory(filePath);

	if (dir != nullptr)
	{
		RE_Path* file = FindPath(filePath, dir);

		if (file != nullptr)
		{
			auto iterPath = dir->tree.begin();
			while (iterPath != dir->tree.end())
			{
				if (*iterPath == file)
				{
					dir->tree.erase(iterPath);
					break;
				}
				iterPath++;
			}

			RE_FileBuffer fileToDelete(filePath, GetZipPath());
			fileToDelete.Delete();
			DEL(file);
		}
		else
			RE_LOG_ERROR("Error deleting file. The file culdn't be located: %s", filePath);
	}
	else
		RE_LOG_ERROR("Error deleting file. The dir culdn't be located: %s", filePath);
}

void RE_FileSystem::DeleteResourceFiles(ResourceContainer* resContainer)
{
	if (resContainer->GetType() != R_SHADER) DeleteUndefinedFile(resContainer->GetAssetPath());
	DeleteUndefinedFile(resContainer->GetMetaPath());

	if (Exists(resContainer->GetLibraryPath()))
	{
		RE_FileBuffer fileToDelete(resContainer->GetLibraryPath(), GetZipPath());
		fileToDelete.Delete();
	}
}

RE_Directory* RE_FileSystem::FindDirectory(const char* pathToFind)
{
	RE_Directory* ret = nullptr;

	eastl::string fullpath(pathToFind);
	eastl::string directoryStr = fullpath.substr(0, fullpath.find_last_of('/') + 1);

	for (auto dir : assetsDirectories)
	{
		if (dir->path == directoryStr)
		{
			ret = dir;
			break;
		}
	}

	return ret;
}

RE_Path* RE_FileSystem::FindPath(const char* pathToFind, RE_Directory* dir)
{
	RE_Path* ret = nullptr;

	eastl::string fullpath(pathToFind);
	eastl::string directoryStr = fullpath.substr(0, fullpath.find_last_of('/') + 1);

	if (!dir) dir = FindDirectory(pathToFind);
	if (dir)
	{
		for (auto file : dir->tree)
		{
			if (file->path == fullpath)
			{
				ret = file;
				break;
			}
		}
	}

	return ret;
}

signed long long RE_FileSystem::GetLastTimeModified(const char* path)
{
	PHYSFS_Stat stat;
	eastl::string full_path(path);
	eastl::string directory = full_path.substr(0, full_path.find_last_of('\\') + 1);
	eastl::string fileNameExtension = full_path.substr(full_path.find_last_of("\\") + 1);

	eastl::string tempPath("/Check/");
	if (PHYSFS_mount(directory.c_str(), "/Check/", 1) != 0)
	{
		tempPath += fileNameExtension;
		if (PHYSFS_stat(tempPath.c_str(), &stat) != 0)
		{
			PHYSFS_removeFromSearchPath(directory.c_str());
			return stat.modtime;
		}
		PHYSFS_removeFromSearchPath(directory.c_str());
	}

	return 0;
}

RE_Json* RE_FileSystem::GetConfigNode(const char* node)
{
	return config->GetRootNode(node);;
}

void RE_FileSystem::SaveConfig()
{
	config->Save();
}

void RE_FileSystem::Internal::CopyDirectory(const char* origin, const char* dest)
{
	eastl::stack<eastl::pair< eastl::string, eastl::string>> copyDir;
	copyDir.push({ origin , dest });

	while (!copyDir.empty()) {

		eastl::string origin(copyDir.top().first);
		eastl::string dest(copyDir.top().second);
		copyDir.pop();

		char** rc = PHYSFS_enumerateFiles(origin.c_str());
		for (char** i = rc; *i != NULL; i++)
		{
			eastl::string inOrigin = origin + *i;
			eastl::string inDest(dest);

			inDest += *i;

			PHYSFS_Stat fileStat;
			if (PHYSFS_stat(inOrigin.c_str(), &fileStat))
			{
				if (fileStat.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY)
				{
					inOrigin += "/";
					inDest += "/";
					copyDir.push({ inOrigin, inDest });
				}
				else if (fileStat.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_REGULAR)
				{
					RE_FileBuffer fileOrigin(inOrigin.c_str());
					if (fileOrigin.Load())
					{
						RE_FileBuffer fileDest(inDest.c_str(), GetZipPath());
						fileDest.Save(fileOrigin.GetBuffer(), fileOrigin.GetSize());
					}
				}
			}
		}
		PHYSFS_freeList(rc);
	}
}
