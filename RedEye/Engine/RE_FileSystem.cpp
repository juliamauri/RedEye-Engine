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

#include "RE_Directory.hpp"

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
		pref_directory = PHYSFS_getPrefDir("RedEye", "RedEye Engine");

		//Loads from argument (RedLens)
		if (argv[1])
		{
			project_path = argv[1];
			project_path = project_path.substr(0, project_path.find_last_of('\\') + 1);
		}
		if (PHYSFS_mount("EngineAssets/", "Internal/", 0) == 0)
		{
			PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
			return false;
		}

		if (PHYSFS_mount(pref_directory.c_str(), "Pref/", 0) == 0)
		{
			PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
			return false;
		}

		if (PHYSFS_mount((argv[1]) ? project_path.c_str() : ".", 0, 0) == 0) {

			PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
			return false;
		}

		if (PHYSFS_setWriteDir((argv[1]) ? project_path.c_str() : ".") == 0) {

			PHYSFS_ErrorCode _direrr = PHYSFS_getLastErrorCode();
			return false;
		}

		//PHYSFS_mount(zip_path.c_str(), NULL, 0);
		//PHYSFS_mount((zip_path += "data.zip").c_str(), NULL, 0);

		char** i;
		for (i = PHYSFS_getSearchPath(); *i != NULL; i++) RE_LOG("[%s] is in the search path.\n", *i);
		PHYSFS_freeList(*i);

		if (argv[1])
		{
			eastl::string config_file = argv[1];
			config_file = config_file.substr(config_file.find_last_of('\\') + 1);
			config = new Config(config_file.c_str());
			if (!config->Load()) RE_LOG_WARNING("Can't load %s - building module default configuration.", config_file.c_str());
		}
		else
		{
			project_path = GetExecutableDirectory();
			config = new Config("Settings/config.json");
			if (!config->Load()) RE_LOG_WARNING("Can't load Settings/config.json - building module default configuration.");
		}

		ret = true;
	}
	else RE_LOG_ERROR("PhysFS could not initialize! Error: %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

	return ret;
}

void RE_FileSystem::Clear()
{
	DEL(config);

	PHYSFS_deinit();
}

void RE_FileSystem::DrawEditor()
{
	ImGui::Text("Executable Directory:");
	ImGui::TextWrappedV(GetExecutableDirectory(), "");

	ImGui::Separator();
	ImGui::Text("Project Directory:");
	ImGui::TextWrappedV(project_path.c_str(), "");

	ImGui::Separator();
	ImGui::Text("All assets directories:");
	for (auto dir : RE_Directory::Assets::GetDirectories())ImGui::Text(dir->path.c_str());
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

void RE_FileSystem::DeleteUndefinedFile(const char* filePath)
{
	RE_FileBuffer fileToDelete(filePath);
	fileToDelete.Delete();

	RE_Directory::Assets::DeleteFile(filePath);
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