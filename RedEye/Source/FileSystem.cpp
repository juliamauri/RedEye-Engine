#include "FileSystem.h"

#include "Globals.h"
#include "SDL2\include\SDL.h"
#include "SDL2\include\SDL_assert.h"

#include "RapidJson\include\rapidjson.h"
#include "RapidJson\include\allocators.h"
#include "RapidJson\include\pointer.h"
#include "RapidJson\include\filereadstream.h"
//#include "RapidJson\include\stringbuffer.h"
//#include "RapidJson\include\writer.h"
//#include "RapidJson\include\filewritestream.h"

#include "PhysFS\include\physfs.h"

#pragma comment( lib, "PhysFS/libx86/physfs.lib" )

FileSystem::FileSystem() : engine_config(nullptr)
{}

FileSystem::~FileSystem()
{
	delete engine_config;
	PHYSFS_deinit();
}

bool FileSystem::Init(int argc, char* argv[])
{
	bool ret = false;

	if (PHYSFS_init(argv[0]) != 0)
	{
		PHYSFS_setWriteDir(".");

		std::string path(GetExecutableDirectory());
		path += "\\Assets";

		AddPath(".");
		AddPath(path.c_str());

		engine_config = new Config("Assets\\config.json");
		if (engine_config->Load())
		{
			ret = true;
		}
		else
		{
			//TODO: Log error - cant load "Config.json"
		}
	}
	else
	{
		//TODO: Log error - cant init physfs
	}

	return ret;

	

	/* TODO:
	PHYSFS_setWriteDir(".");
	AddPath(".");*/
}

Config* FileSystem::GetConfig() const
{
	return engine_config;
}

bool FileSystem::AddPath(const char * path_or_zip, const char * mount_point)
{
	bool ret = true;

	if (PHYSFS_mount(path_or_zip, mount_point, 1) == 0)
	{
		LOG("File System error while adding a path or zip(%s): %s\n", path_or_zip, PHYSFS_getLastError());
		ret = false;
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
	if (buffer != nullptr)//delete buffer;
		memset(buffer, 0, size);
}

bool RE_FileIO::Load()
{
	return HardLoad() > 0;
}

void RE_FileIO::Save()
{
	//TODO: save buffer to file
}

void RE_FileIO::ClearBuffer()
{
	memset(buffer, 0, sizeof buffer);
	buffer = nullptr;
}

const char* RE_FileIO::GetBuffer() const
{
	return (buffer);
}

inline bool RE_FileIO::operator!() const
{
	return buffer == nullptr;
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
			size = (unsigned int)sll_size;
			if (sll_size > 0)
			{
				buffer = new char[(unsigned int)sll_size];
				signed long long amountRead = PHYSFS_read(fs_file, buffer, 1, (signed int)sll_size);
				
				if (amountRead != sll_size)
				{
					LOG("File System error while reading from file %s: %s\n", file_name, PHYSFS_getLastError());
					delete (buffer);
				}
				else
				{
					ret = (uint)amountRead;
					buffer[ret] = '\0';
				}
			}

			if (PHYSFS_close(fs_file) == 0)
			{
				LOG("File System error while closing file %s: %s\n", file_name, PHYSFS_getLastError());
			}
		}
		else
		{
			LOG("File System error while opening file %s: %s\n", file_name, PHYSFS_getLastError());
		}
	}
	else
	{
		LOG("File System error while opening file %s: %s\n", file_name, PHYSFS_getLastError());
	}

	return ret;
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
	FILE* fp = nullptr;
	std::string full_path(SDL_GetBasePath());
	full_path += file_name;
	if (fopen_s(&fp, full_path.c_str(), "rb") == 0);// non-Windows use "r"
	{
		// Read File
		char readBuffer[65535];
		rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

		// Parse file buffer into root::rapidjson::Document
		//document.SetNull();
		document.ParseStream(is);

		// Close file
		fclose(fp);

		ret = true;
	}
	return ret;
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
			value = &rapidjson::Pointer(path.c_str()).Create(document);

		ret = new JSONNode(path.c_str(), this);
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

JSONNode::JSONNode(JSONNode& node) : pointerPath(nullptr), config(nullptr)
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
	uint ret = 0;

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

//JSONNode PullJObject(const char* name) const;

inline bool JSONNode::operator!() const
{
	return (config == nullptr) || pointerPath.empty();
}