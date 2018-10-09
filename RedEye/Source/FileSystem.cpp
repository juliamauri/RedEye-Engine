#include "FileSystem.h"

#include "Application.h"
#include "OutputLog.h"
#include "Globals.h"
#include "ImGui\imgui.h"
#include "SDL2\include\SDL.h"
#include "SDL2\include\SDL_assert.h"

#include "RapidJson\include\rapidjson.h"
#include "RapidJson\include\allocators.h"
#include "RapidJson\include\pointer.h"
#include "RapidJson\include\stringbuffer.h"
#include "RapidJson\include\writer.h"

#include "PhysFS\include\physfs.h"

#pragma comment( lib, "PhysFS/libx86/physfs.lib" )

FileSystem::FileSystem() : engine_config(nullptr)
{}

FileSystem::~FileSystem()
{
	paths.clear();
	delete engine_config;

	PHYSFS_deinit();
}

bool FileSystem::Init(int argc, char* argv[])
{
	bool ret = false;

	if (PHYSFS_init(argv[0]) != 0)
	{
		PHYSFS_Version physfs_version;
		PHYSFS_VERSION(&physfs_version);
		char tmp[8];
		sprintf_s(tmp, 8, "%u.%u.%u", (int)physfs_version.major, (int)physfs_version.minor, (int)physfs_version.patch);
		App->ReportSoftware("PhysFS", tmp, "https://icculus.org/physfs/");
		App->ReportSoftware("Rapidjson", RAPIDJSON_VERSION_STRING, "http://rapidjson.org/");
		
		AddPath(".");
		std::string path(GetExecutableDirectory()); path += "\\Assets";
		AddPath(path.c_str());

		SetWritePath(path.c_str());

		const char* config_file = "Settings/config.json";
		engine_config = new Config(config_file);
		if (engine_config->Load())
			ret = true;
		else
			LOG("Error while loading Engine Configuration file: %s\nRed Eye Engine will initialize with default configuration parameters.", config_file);
	}
	else
	{
		LOG("PhysFS could not initialize! Error: %s\n", PHYSFS_getLastError());
	}

	return ret;
}

Config* FileSystem::GetConfig() const
{
	return engine_config;
}

void FileSystem::DrawEditor()
{
	ImGui::Text("Executable Directory:");
	ImGui::TextWrappedV(GetExecutableDirectory(), "");

	ImGui::Separator();

	ImGui::Text("Read Directories");
	for (std::list<std::string>::iterator it = paths.begin(); it != paths.end(); ++it)
		ImGui::TextWrappedV(it->c_str(), "");

	ImGui::Separator();

	ImGui::Text("Write Directory");
	ImGui::TextWrappedV(write_path.c_str(), "");
}

bool FileSystem::AddPath(const char * path_or_zip, const char * mount_point)
{
	bool ret = true;

	if (PHYSFS_mount(path_or_zip, mount_point, 1) == 0)
	{
		LOG("File System error while adding a path or zip(%s): %s\n", path_or_zip, PHYSFS_getLastError());
		ret = false;
	}
	else
	{
		paths.push_back(path_or_zip);
	}

	return ret;
}

bool FileSystem::RemovePath(const char * path_or_zip)
{
	bool ret = true;

	if (PHYSFS_removeFromSearchPath(path_or_zip) == 0)
	{
		LOG("Error removing PhysFS Directory (%s): %s", path_or_zip, PHYSFS_getLastError());
		ret = false;
	}

	paths.remove(path_or_zip);

	return ret;
}

bool FileSystem::SetWritePath(const char * dir)
{
	bool ret = true;

	if (!PHYSFS_setWriteDir(dir))
	{
		LOG("Error setting PhysFS Directory: %s", PHYSFS_getLastError());
		ret = false;
	}
	else
	{
		write_path = dir;
	}

	return ret;
}

const char * FileSystem::GetWritePath() const
{
	return write_path.c_str();
}

// Quick Buffer From Platform-Dependent Path
RE_FileIO* FileSystem::QuickBufferFromPDPath(const char * full_path)// , char** buffer, unsigned int size)
{
	RE_FileIO* ret = nullptr;

	if (full_path != nullptr)
	{
		std::string file_path = full_path;
		std::string file_name = file_path.substr(file_path.find_last_of("\\") + 1);
		std::string ext = file_name.substr(file_name.find_last_of(".") + 1);
		file_path.erase(file_path.length() - file_name.length(), file_path.length());

		ret = new RE_FileIO(file_name.c_str());
		if (App->fs->AddPath(file_path.c_str()))
		{
			if (!ret->Load()) DEL(ret);
			App->fs->RemovePath(file_path.c_str());
		}
		else
		{
			DEL(ret);
		}
	}

	return ret;
}

bool FileSystem::Exists(const char* file) const
{
	return PHYSFS_exists(file) != 0;
}

bool FileSystem::IsDirectory(const char* file) const
{
	return PHYSFS_isDirectory(file) != 0;
}

const char* FileSystem::GetExecutableDirectory() const
{
	return PHYSFS_getBaseDir();
}


////////////////////////////////////////////////////////////////////
///////////////////RE_FileIO
////////////////////////////////////////////////////////////////////

RE_FileIO::RE_FileIO(const char* file_name) : buffer(nullptr), file_name(file_name) {}

RE_FileIO::~RE_FileIO()
{
	DEL_A(buffer);
}

bool RE_FileIO::Load()
{
	size = HardLoad();
	return size > 0;
}

void RE_FileIO::Save()
{
	HardSave(buffer);
}

void RE_FileIO::ClearBuffer()
{
	delete[] buffer;
	buffer = nullptr;
}

char * RE_FileIO::GetBuffer()
{
	return (buffer);
}

const char* RE_FileIO::GetBuffer() const
{
	return (buffer);
}

inline bool RE_FileIO::operator!() const
{
	return buffer == nullptr;
}

unsigned int RE_FileIO::GetSize()
{
	return size;
}

unsigned int RE_FileIO::HardLoad()
{
	unsigned int ret = 0u;

	if (PHYSFS_exists(file_name))
	{
		PHYSFS_File* fs_file = PHYSFS_openRead(file_name);

		if (fs_file != NULL)
		{
			signed long long sll_size = PHYSFS_fileLength(fs_file);
			if (sll_size > 0)
			{
				buffer = new char[(unsigned int)sll_size + 1];
				signed long long amountRead = PHYSFS_read(fs_file, buffer, 1, (signed int)sll_size);
				
				if (amountRead != sll_size)
				{
					LOG("File System error while reading from file %s: %s", file_name, PHYSFS_getLastError());
					delete (buffer);
				}
				else
				{
					ret = (unsigned int)amountRead;
					buffer[ret] = '\0';
				}
			}

			if (PHYSFS_close(fs_file) == 0)
			{
				LOG("File System error while closing file %s: %s", file_name, PHYSFS_getLastError());
			}
		}
		else
		{
			LOG("File System error while opening file %s: %s", file_name, PHYSFS_getLastError());
		}
	}
	else
	{
		LOG("File System error while checking file %s: %s", file_name, PHYSFS_getLastError());
	}

	return ret;
}

void RE_FileIO::HardSave(const char* buffer)
{
	PHYSFS_file* file = PHYSFS_openWrite(file_name);

	if (file != NULL)
	{
		long long written = PHYSFS_write(file, (const void*)buffer, 1, size = (strnlen_s(buffer, 0xffff)));
		if (written != size)
		{
			LOG("Error while writing to file %s: %s", file, PHYSFS_getLastError());
		}

		if (PHYSFS_close(file) == 0)
			LOG("Error while closing save file %s: %s", file, PHYSFS_getLastError());
	}
	else
		LOG("Error while opening save file %s: %s", file, PHYSFS_getLastError());
}


////////////////////////////////////////////////////////////////////
///////////////////Config
////////////////////////////////////////////////////////////////////

//Tutorial http://rapidjson.org/md_doc_tutorial.html

Config::Config(const char* file_name) : RE_FileIO(file_name) {}

bool Config::Load()
{
	// Open file
	bool ret = false;

	size = HardLoad();

	if (ret = (size > 0))
	{
		rapidjson::StringStream s(buffer);
		document.ParseStream(s);
	}

	return ret;
}

void Config::Save()
{
	rapidjson::StringBuffer s_buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s_buffer);
	document.Accept(writer);
	s_buffer.Put('\0');
	size = s_buffer.GetSize();
	const char* output = s_buffer.GetString();
	HardSave(output);
}

JSONNode* Config::GetRootNode(const char* member)
{
	JSONNode* ret = nullptr;

	if (member != nullptr)
	{
		std::string path("/");
		path += member;
		rapidjson::Value* value = rapidjson::Pointer(path.c_str()).Get(document);

		if (value == nullptr)
		{
			value = &rapidjson::Pointer(path.c_str()).Create(document);
			LOG("Configuration node not found for %s, created new pointer", path.c_str());
		}

		ret = new JSONNode(path.c_str(), this);
	}
	else
	{
		LOG("Error Loading Configuration node: Empty Member Name");
	}

	return ret;
}

inline bool Config::operator!() const
{
	return document.IsNull();
}


////////////////////////////////////////////////////////////////////
///////////////////JSONNode
////////////////////////////////////////////////////////////////////

JSONNode::JSONNode(const char* path, Config* config) : pointerPath(path), config(config)
{}

JSONNode::JSONNode(JSONNode& node) : pointerPath(node.pointerPath), config(node.config)
{}

JSONNode::~JSONNode()
{
	config = nullptr;
}


// Push ============================================================

void JSONNode::PushBool(const char* name, const bool value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushInt(const char* name, const int value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushUInt(const char* name, const unsigned int value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushFloat(const char* name, const float value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushDouble(const char* name, const double value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

void JSONNode::PushString(const char* name, const char* value)
{
	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Pointer(path.c_str()).Set(config->document, value);
	}
}

JSONNode* JSONNode::PushJObject(const char* name)
{
	JSONNode* ret = nullptr;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		ret = new JSONNode(path.c_str(), config);
	}

	return ret;
}

// Pull ============================================================

bool JSONNode::PullBool(const char* name, bool deflt)
{
	bool ret = false;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		ret = (val != nullptr) ? val->GetBool() : deflt;
	}

	return ret;
}

int JSONNode::PullInt(const char* name, int deflt)
{
	int ret = 0;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		ret = (val != nullptr) ? val->GetInt() : deflt;
	}

	return ret;
}

unsigned int JSONNode::PullUInt(const char* name, const unsigned int deflt)
{
	unsigned int ret = 0;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		ret = (val != nullptr) ? val->GetUint() : deflt;
	}

	return ret;
}


float JSONNode::PullFloat(const char* name, float deflt)
{
	float ret = 0;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		ret = (val != nullptr) ? val->GetFloat() : deflt;
	}

	return ret;
}

double JSONNode::PullDouble(const char* name, double deflt)
{
	double ret = 0;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);
		ret = (val != nullptr) ? val->GetDouble() : deflt;
	}

	return ret;
}

const char*	JSONNode::PullString(const char* name, const char* deflt)
{
	const char* ret = nullptr;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		rapidjson::Value* val = rapidjson::Pointer(path.c_str()).Get(config->document);

		ret = (val != nullptr) ? val->GetString() : deflt;
	}

	return ret;
}

JSONNode* JSONNode::PullJObject(const char * name)
{
	JSONNode* ret = nullptr;

	if (name != nullptr)
	{
		std::string path(pointerPath);
		path += "/";
		path += name;

		ret = new JSONNode(path.c_str(), config);
	}

	return ret;
}

inline bool JSONNode::operator!() const
{
	return (config == nullptr) || pointerPath.empty();
}

const char * JSONNode::GetDocumentPath() const
{
	return pointerPath.c_str();
}
