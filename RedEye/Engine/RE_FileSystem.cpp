#include "RE_FileSystem.h"

#include "RE_Memory.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"

#include "RE_Profiler.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"

#include "RE_Time.h"
#include "RE_ConsoleLog.h"

#include "RE_ResourceManager.h"
#include "RE_PrimitiveManager.h"
#include "RE_ModelImporter.h"

#include "RE_GameObject.h"
#include "RE_CompTransform.h"
#include "RE_CompMesh.h"
#include "RE_CompPrimitive.h"
#include "RE_Mesh.h"
#include "RE_Shader.h"

#include <ImGui/imgui.h>
#include <PhysFS/physfs.h>

#include <EASTL\internal\char_traits.h>
#include <EASTL\algorithm.h>
#include <EASTL\iterator.h>
#include <EAStdC\EASprintf.h>

bool RE_FileSystem::Init(int argc, char* argv[])
{
	RE_PROFILE(PROF_Init, PROF_FileSystem);
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

		engine_path = "engine";
		library_path = "Library";
		assets_path = "Assets";

		//Loads from argument (RedLens)
		if (PHYSFS_mount((argv[1]) ? argv[1] : ".", 0, 0) == 0) {

			PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
			return false;
		}
		if (PHYSFS_setWriteDir((argv[1]) ? argv[1] : ".") == 0) {

			PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
			return false;
		}

		//PHYSFS_mount(zip_path.c_str(), NULL, 0);
		//PHYSFS_mount((zip_path += "data.zip").c_str(), NULL, 0);

		char** i;
		for (i = PHYSFS_getSearchPath(); *i != NULL; i++) RE_LOG("[%s] is in the search path.\n", *i);
		PHYSFS_freeList(*i);

		config = new Config("Settings/config.json");
		if (!config->Load()) RE_LOG_WARNING("Can't load Settings/config.json - building module default configuration.");

		rootAssetDirectory = new RE_Directory();
		rootAssetDirectory->SetPath("Assets/");
		assetsDirectories = rootAssetDirectory->MountTreeFolders();
		assetsDirectories.push_front(rootAssetDirectory);
		dirIter = assetsDirectories.begin();

		ret = true;
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
	RE_PROFILE(PROF_ReadAssetChanges, PROF_FileSystem);
	RE_Timer time;
	bool run = true;

	if ((dirIter == assetsDirectories.begin()) && (!assets_to_process.empty() || !filesToFindMeta.empty() || !toImport.empty() || !toReImport.empty()))
		dirIter = assetsDirectories.end();

	while ((doAll || run) && dirIter != assetsDirectories.end())
	{
		eastl::stack<RE_ProcessPath*> toProcess = (*dirIter)->CheckAndApply(&metaRecentlyAdded);
		while (!toProcess.empty())
		{
			RE_ProcessPath* process = toProcess.top();
			toProcess.pop();
			assets_to_process.push(process);
		}

		dirIter++;

		//timer
		if (!doAll && extra_ms < time.Read()) run = false;
	}

	if (dirIter == assetsDirectories.end())
	{
		dirIter = assetsDirectories.begin();

		while ((doAll || run) && !assets_to_process.empty())
		{
			RE_ProcessPath* process = assets_to_process.top();
			assets_to_process.pop();

			switch (process->procedure)
			{
			case P_ADDFILE:
			{
				RE_File* file = process->toProcess->AsFile();
				RE_Meta* metaFile = file->AsMeta();

				FileType type = file->fType;

				if (type == F_META)
				{
					const char* isReferenced = RE_RES->FindMD5ByMETAPath(file->path.c_str());

					if (!isReferenced)
					{
						Config metaLoad(file->path.c_str());
						if (metaLoad.Load())
						{
							RE_Json* metaNode = metaLoad.GetRootNode("meta");
							Resource_Type type = (Resource_Type)metaNode->PullInt("Type", Resource_Type::R_UNDEFINED);
							DEL(metaNode);

							if (type == Resource_Type::R_PARTICLE_EMITTER) {
								meta_to_process_last.push(process);
								continue;
							}

							if (RE_RES->isNeededResoursesLoaded(file->path.c_str(), type))
								reloadResourceMeta.push(metaFile);

							if (type != Resource_Type::R_UNDEFINED)
								metaFile->resource = RE_RES->ReferenceByMeta(file->path.c_str(), type);
						}
					}
					else
						metaFile->resource = isReferenced;

					metaToFindFile.push_back(metaFile);
				}
				else if (type > F_NONE) filesToFindMeta.push_back(file);
				else RE_LOG_ERROR("Unsupported file type");
				break;
			}
			case P_ADDFOLDER:
			{
				RE_Directory* dir = (RE_Directory*)process->toProcess;
				eastl::stack<RE_ProcessPath*> toProcess = dir->CheckAndApply(&metaRecentlyAdded);

				while (!toProcess.empty())
				{
					RE_ProcessPath* process = toProcess.top();
					toProcess.pop();
					assets_to_process.push(process);
				}

				assetsDirectories.push_back(dir);
				break;
			}
			case P_REIMPORT:
			{
				toReImport.push(process->toProcess->AsFile());
				break;
			}
			}
			DEL(process);

			//timer
			if (!doAll && extra_ms < time.Read()) run = false;
		}

		while ((doAll || run) && !meta_to_process_last.empty())
		{
			RE_ProcessPath* process = meta_to_process_last.top();
			meta_to_process_last.pop();

			RE_File* file = process->toProcess->AsFile();
			RE_Meta* metaFile = file->AsMeta();

			metaFile->resource = RE_RES->ReferenceByMeta(file->path.c_str(), Resource_Type::R_PARTICLE_EMITTER);

			//timer
			if (!doAll && extra_ms < time.Read()) run = false;
		}

		if ((doAll || run) && !metaToFindFile.empty())
		{
			eastl::vector<RE_File*> toRemoveF;
			eastl::vector<RE_Meta*> toRemoveM;

			for (RE_Meta* meta : metaToFindFile)
			{
				ResourceContainer* res = RE_RES->At(meta->resource);
				const char* assetPath = RE_RES->At(meta->resource)->GetAssetPath();

				if (!res->isInternal())
				{
					int sizefile = 0;
					if (!filesToFindMeta.empty())
					{
						for (RE_File* file : filesToFindMeta)
						{
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

		if (!filesToFindMeta.empty())
		{
			for (RE_File* file : filesToFindMeta)
			{
				toImport.push(file);
			}
			filesToFindMeta.clear();
		}

		if (!toImport.empty()) {
			RE_LOGGER.ScopeProcedureLogging();
			while ((doAll || run) && !toImport.empty())
			{

				RE_File* file = toImport.top();
				toImport.pop();

				//Importing
				RE_LOG("Importing %s", file->path.c_str());

				const char* newRes = nullptr;
				switch (file->fType) {
				case F_MODEL:	 newRes = RE_RES->ImportModel(file->path.c_str()); break;
				case F_TEXTURE:	 newRes = RE_RES->ImportTexture(file->path.c_str()); break;
				case F_MATERIAL: newRes = RE_RES->ImportMaterial(file->path.c_str()); break;
				case F_SKYBOX:	 newRes = RE_RES->ImportSkyBox(file->path.c_str()); break;
				case F_PREFAB:	 newRes = RE_RES->ImportPrefab(file->path.c_str()); break;
				case F_SCENE:	 newRes = RE_RES->ImportScene(file->path.c_str()); break;
				case F_PARTICLEEMISSOR:	 newRes = RE_RES->ImportParticleEmissor(file->path.c_str()); break;
				case F_PARTICLERENDER:	 newRes = RE_RES->ImportParticleRender(file->path.c_str()); break;
				}

				if (newRes != nullptr)
				{
					RE_Meta* newMetaFile = new RE_Meta();
					newMetaFile->resource = newRes;
					newMetaFile->fromFile = file;
					file->metaResource = newMetaFile;
					RE_File* fromMetaF = newMetaFile->AsFile();
					fromMetaF->fType = FileType::F_META;
					fromMetaF->path = RE_RES->At(newRes)->GetMetaPath();
					newMetaFile->AsPath()->pType = PathType::D_FILE;
					metaRecentlyAdded.push_back(newMetaFile);
				}
				else
					file->fType = F_NOTSUPPORTED;

				if (!doAll && extra_ms < time.Read())
				{
					run = false;
					break;
				}
			}
			RE_LOGGER.EndScope();
		}

		while ((doAll || run) && !reloadResourceMeta.empty())
		{
			RE_Meta* metaFile = reloadResourceMeta.top();
			reloadResourceMeta.pop();

			RE_RES->At(metaFile->resource)->LoadMeta();

			//timer
			if (!doAll && extra_ms < time.Read()) run = false;
		}

		if (!toReImport.empty())
		{
			static bool particle_reimport = false;
			while ((doAll || run) && !toReImport.empty())
			{
				RE_Meta* meta = toReImport.top()->AsMeta();
				toReImport.pop();

				RE_LOG("ReImporting %s", meta->path.c_str());

				switch (RE_RES->At(meta->resource)->GetType())
				{
				case R_SHADER:
					RE_RES->At(meta->resource)->ReImport();
					break;
				case R_PARTICLE_RENDER:
				case R_PARTICLE_EMISSION:
					RE_RES->PushParticleResource(meta->resource);
					particle_reimport = true;
					break;
				}

				if (!doAll && extra_ms < time.Read())
				{
					run = false;
					break;
				}
			}
			if (particle_reimport) {
				RE_RES->ProcessParticlesReimport();
				particle_reimport = false;
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

bool RE_FileSystem::AddPath(const char* path_or_zip, const char* mount_point)
{
	bool ret = true;

	if (PHYSFS_mount(path_or_zip, mount_point, 1) == 0)
	{
		RE_LOG_ERROR("File System error while adding a path or zip(%s): %s\n", path_or_zip, PHYSFS_getLastError());
		ret = false;
	}
	return ret;
}

bool RE_FileSystem::RemovePath(const char* path_or_zip)
{
	bool ret = true;

	if (PHYSFS_removeFromSearchPath(path_or_zip) == 0)
	{
		RE_LOG_ERROR("Error removing PhysFS Directory (%s): %s", path_or_zip, PHYSFS_getLastError());
		ret = false;
	}
	return ret;
}

bool RE_FileSystem::SetWritePath(const char* dir)
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

const char* RE_FileSystem::GetWritePath() const { return write_path.c_str(); }

void RE_FileSystem::LogFolderItems(const char* folder)
{
	char** rc = PHYSFS_enumerateFiles(folder), ** i;
	for (i = rc; *i != NULL; i++) RE_LOG(" * We've got [%s].\n", *i);
	PHYSFS_freeList(rc);
}

// Quick Buffer From Platform-Dependent Path
RE_FileBuffer* RE_FileSystem::QuickBufferFromPDPath(const char* full_path)// , char** buffer, unsigned int size)
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

		if (RE_FS->AddPath(file_path.c_str(), "/Export/"))
		{
			if (!ret->Load()) DEL(ret);
			RE_FS->RemovePath(file_path.c_str());
		}
		else DEL(ret);
	}

	return ret;
}

bool RE_FileSystem::ExistsOnOSFileSystem(const char* path, bool isFolder) const
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

bool RE_FileSystem::Exists(const char* file) const { return PHYSFS_exists(file) != 0; }
bool RE_FileSystem::IsDirectory(const char* file) const { return PHYSFS_isDirectory(file) != 0; }
const char* RE_FileSystem::GetExecutableDirectory() const { return PHYSFS_getBaseDir(); }

void RE_FileSystem::HandleDropedFile(const char* file)
{
	RE_PROFILE(PROF_DroppedFile, PROF_FileSystem);
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
			eastl::string destPath = RE_EDITOR->GetAssetsPanelPath();
			destPath += fileNameExtension;
			RE_FileBuffer toSave(destPath.c_str());
			toSave.Save(fileLoaded->GetBuffer(), fileLoaded->GetSize());
		}
		else newDir = true;
	}

	if (newDir)
	{
		finalPath += fileName + "/";
		RE_FS->AddPath(file, finalPath.c_str());
		CopyDirectory(exportPath.c_str(), RE_EDITOR->GetAssetsPanelPath());
		RE_FS->RemovePath(file);
	}
}

RE_FileSystem::RE_Directory* RE_FileSystem::GetRootDirectory() const { return rootAssetDirectory; }

void RE_FileSystem::DeleteUndefinedFile(const char* filePath)
{
	RE_FileSystem::RE_Directory* dir = FindDirectory(filePath);

	if (dir != nullptr)
	{
		RE_FileSystem::RE_Path* file = FindPath(filePath, dir);

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

			RE_FileBuffer fileToDelete(filePath);
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
	if (resContainer->GetType() != R_SHADER && resContainer->GetType() != R_PARTICLE_EMITTER) DeleteUndefinedFile(resContainer->GetAssetPath());
	DeleteUndefinedFile(resContainer->GetMetaPath());

	if (Exists(resContainer->GetLibraryPath()))
	{
		RE_FileBuffer fileToDelete(resContainer->GetLibraryPath());
		fileToDelete.Delete();
	}
}

RE_FileSystem::RE_Directory* RE_FileSystem::FindDirectory(const char* pathToFind)
{
	RE_FileSystem::RE_Directory* ret = nullptr;

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

RE_FileSystem::RE_Path* RE_FileSystem::FindPath(const char* pathToFind, RE_Directory* dir)
{
	RE_FileSystem::RE_Path* ret = nullptr;

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

RE_Json* RE_FileSystem::ConfigNode(const char* node) const
{
	return (config != nullptr && node != nullptr) ? config->GetRootNode(node) : nullptr;
}

void RE_FileSystem::SaveConfig() const
{
	config->Save();
}

void RE_FileSystem::CopyDirectory(const char* origin, const char* dest)
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
						RE_FileBuffer fileDest(inDest.c_str());
						fileDest.Save(fileOrigin.GetBuffer(), fileOrigin.GetSize());
					}
				}
			}
		}
		PHYSFS_freeList(rc);
	}
}

RE_FileSystem::FileType RE_FileSystem::RE_File::DetectExtensionAndType(const char* _path, const char*& _extension)
{
	static const char* extensionsSuported[14] = { "meta", "re","refab", "pupil", "sk",  "fbx", "jpg", "dds", "png", "tga", "tiff", "bmp", "lasse", "lopfe" };

	RE_FileSystem::FileType ret = F_NOTSUPPORTED;
	eastl::string modPath(_path);
	eastl::string filename = modPath.substr(modPath.find_last_of("/") + 1);
	eastl::string extensionStr = filename.substr(filename.find_last_of(".") + 1);
	eastl::transform(extensionStr.begin(), extensionStr.end(), extensionStr.begin(), [](unsigned char c) { return eastl::CharToLower(c); });

	for (uint i = 0; i < 14; i++)
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
			case 12: ret = F_PARTICLEEMISSOR; break;
			case 13: ret = F_PARTICLERENDER; break;
			}
		}
	}
	return ret;
}

bool RE_FileSystem::RE_Meta::IsModified() const
{
	return (RE_RES->At(resource)->GetType() != Resource_Type::R_SHADER
		&& fromFile->lastModified != RE_RES->At(resource)->GetLastTimeModified());
}

void RE_FileSystem::RE_Directory::SetPath(const char* _path)
{
	path = _path;
	name = eastl::string(path.c_str(), path.size() - 1);
	name = name.substr(name.find_last_of("/") + 1);
}

eastl::list< RE_FileSystem::RE_Directory*> RE_FileSystem::RE_Directory::MountTreeFolders()
{
	eastl::list< RE_Directory*> ret;
	if (RE_FS->Exists(path.c_str()))
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
				tree.push_back(newDirectory->AsPath());
				ret.push_back(newDirectory);
			}
		}
		PHYSFS_freeList(rc);

		if (!tree.empty())
		{
			for (RE_Path* path : tree)
			{
				eastl::list< RE_Directory*> fromChild = path->AsDirectory()->MountTreeFolders();
				if (!fromChild.empty()) ret.insert(ret.end(), fromChild.begin(), fromChild.end());
			}
		}
	}
	return ret;
}

eastl::stack<RE_FileSystem::RE_ProcessPath*> RE_FileSystem::RE_Directory::CheckAndApply(eastl::vector<RE_Meta*>* metaRecentlyAdded)
{
	eastl::stack<RE_ProcessPath*> ret;
	if (RE_FS->Exists(path.c_str()))
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

						AddBeforeOf(newDirectory->AsPath(), iter);
						iter--;

						RE_ProcessPath* newProcess = new RE_ProcessPath();
						newProcess->procedure = P_ADDFOLDER;
						newProcess->toProcess = newDirectory->AsPath();
						ret.push(newProcess);
					}
				}
				else if (fileStat.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_REGULAR)
				{
					const char* extension = nullptr;
					FileType fileType = RE_File::DetectExtensionAndType(inPath.c_str(), extension);
					bool newFile = (iterTreeType == PathType::D_NULL || iterTreeType != PathType::D_FILE || (*iter)->path != inPath);
					if (newFile && fileType == FileType::F_META && !metaRecentlyAdded->empty())
					{
						int sizemetap = 0;
						for (RE_Meta* metaAdded : *metaRecentlyAdded)
						{
							sizemetap = eastl::CharStrlen(metaAdded->path.c_str());
							if (sizemetap > 0 && eastl::Compare(metaAdded->path.c_str(), inPath.c_str(), sizemetap) == 0)
							{
								AddBeforeOf(metaAdded->AsPath(), iter);
								iter--;
								toRemoveM.push_back(metaAdded);
								metaRecentlyAdded->erase(eastl::remove_if(eastl::begin(*metaRecentlyAdded), eastl::end(*metaRecentlyAdded),
									[&](auto x) {return eastl::find(begin(toRemoveM), end(toRemoveM), x) != end(toRemoveM); }), eastl::end(*metaRecentlyAdded));
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
						RE_File* newFile = (fileType != FileType::F_META) ? new RE_File() : (RE_File*)new RE_Meta();
						newFile->path = inPath;
						newFile->filename = inPath.substr(inPath.find_last_of("/") + 1);
						newFile->lastSize = fileStat.filesize;
						newFile->pType = PathType::D_FILE;
						newFile->fType = fileType;
						newFile->extension = extension;
						newFile->lastModified = fileStat.modtime;

						RE_ProcessPath* newProcess = new RE_ProcessPath();
						newProcess->procedure = P_ADDFILE;
						newProcess->toProcess = newFile->AsPath();
						ret.push(newProcess);
						AddBeforeOf(newFile->AsPath(), iter);
						iter--;
					}
					else if ((*iter)->AsFile()->fType == FileType::F_META)
					{
						bool reimport = false;
						ResourceContainer* res = RE_RES->At((*iter)->AsFile()->AsMeta()->resource);
						switch (res->GetType())
						{
						case R_SHADER: {
							RE_Shader* shadeRes = dynamic_cast<RE_Shader*>(res);
							if (shadeRes->isShaderFilesChanged())
							{
								RE_ProcessPath* newProcess = new RE_ProcessPath();
								newProcess->procedure = P_REIMPORT;
								newProcess->toProcess = (*iter);
								ret.push(newProcess);
							}
							break; }
						case R_PARTICLE_RENDER:
						case R_PARTICLE_EMISSION: {

							if ((*iter)->AsFile()->lastModified != fileStat.modtime) {
								(*iter)->AsFile()->lastModified = fileStat.modtime;

								RE_ProcessPath* newProcess = new RE_ProcessPath();
								newProcess->procedure = P_REIMPORT;
								newProcess->toProcess = (*iter);
								ret.push(newProcess);
							}
							break;
						}
						}
					}
				}
			}
			if (iter != tree.end()) iter++;
		}
		PHYSFS_freeList(rc);
	}
	return ret;
}

eastl::stack<RE_FileSystem::RE_Path*> RE_FileSystem::RE_Directory::GetDisplayingFiles() const
{
	eastl::stack<RE_FileSystem::RE_Path*> ret;

	for (auto path : tree)
	{
		switch (path->pType)
		{
		case RE_FileSystem::PathType::D_FILE:
		{
			switch (path->AsFile()->fType)
			{
			case RE_FileSystem::FileType::F_META:
			{
				if (path->AsMeta()->resource)
				{
					ResourceContainer* res = RE_RES->At(path->AsMeta()->resource);
					if (res->GetType() == R_SHADER || res->GetType() == R_PARTICLE_EMITTER) ret.push(path);
				}
				break;
			}
			case RE_FileSystem::FileType::F_NONE: break;
			default: ret.push(path); break;
			}
			break;
		}
		case RE_FileSystem::PathType::D_FOLDER: ret.push(path); break;
		}
	}

	return ret;
}

eastl::list<RE_FileSystem::RE_Directory*> RE_FileSystem::RE_Directory::FromParentToThis()
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
