#include "FileSystem.h"

#include "Globals.h"
#include "SDL2\include\SDL.h"
#include "SDL2\include\SDL_assert.h"

#include "RapidJson\include\rapidjson.h"
#include "RapidJson\include\allocators.h"
#include "RapidJson\include\pointer.h"
#include "RapidJson\include\stringbuffer.h"
#include "RapidJson\include\writer.h"
#include "RapidJson\include\filewritestream.h"
#include "RapidJson\include\filereadstream.h"

#include "PhysFS\include\physfs.h"

#pragma comment( lib, "PhysFS/libx86/physfs.lib" )

////////////////////////////////////////////////////////////////////
///////////////////JSONNode
////////////////////////////////////////////////////////////////////

JSONNode::JSONNode(const char* path, Config* config) : pointerPath(path), config(config)
{}

JSONNode::~JSONNode()
{
	config = nullptr;
}

// Push ============================================================

/*bool PushBool(const char* name, const bool value);
bool PushInt(const char* name, const int value);
bool PushUInt(const char* name, const uint value);
bool PushFloat(const char* name, const float value);
bool PushDouble(const char* name, const double value);*/

void JSONNode::PushString(const char* name, const char* value)
{
	if (name != nullptr)
	{
		char buffer[RAPIDJSON_MAX_PATH_BUFFER];
		int len = sprintf_s(buffer, "%s/%s", pointerPath, name);
		rapidjson::Pointer(buffer).Set(config->document, value);
		memset(buffer, 0, sizeof(buffer));
	}
}

//JSONNode PushJObject(const char* name);

// Pull ============================================================

bool JSONNode::PullBool(const char* name, bool deflt)
{
	bool ret = false;

	if (name != nullptr)
	{
		char buffer[RAPIDJSON_MAX_PATH_BUFFER];
		int len = sprintf_s(buffer, "%s/%s", pointerPath, name);
		rapidjson::Value* val = rapidjson::Pointer(buffer).Get(config->document);
		ret = (val != nullptr) ? val->GetFloat() : deflt;
		memset(buffer, 0, sizeof(buffer));
	}

	return ret;
}

int JSONNode::PullInt(const char* name, int deflt)
{
	int ret = 0;

	if (name != nullptr)
	{
		char buffer[RAPIDJSON_MAX_PATH_BUFFER];
		int len = sprintf_s(buffer, "%s/%s", pointerPath, name);
		rapidjson::Value* val = rapidjson::Pointer(buffer).Get(config->document);
		ret = (val != nullptr) ? val->GetInt() : deflt;
		memset(buffer, 0, sizeof(buffer));
	}

	return ret;
}

uint JSONNode::PullUInt(const char* name, uint deflt)
{
	uint ret = 0;

	if (name != nullptr)
	{
		char buffer[RAPIDJSON_MAX_PATH_BUFFER];
		int len = sprintf_s(buffer, "%s/%s", pointerPath, name);
		rapidjson::Value* val = rapidjson::Pointer(buffer).Get(config->document);
		ret = (val != nullptr) ? val->GetUint() : deflt;
		memset(buffer, 0, sizeof(buffer));
	}

	return ret;
}


float JSONNode::PullFloat(const char* name, float deflt)
{
	float ret = 0;

	if (name != nullptr)
	{
		char buffer[RAPIDJSON_MAX_PATH_BUFFER];
		int len = sprintf_s(buffer, "%s/%s", pointerPath, name);
		rapidjson::Value* val = rapidjson::Pointer(buffer).Get(config->document);
		ret = (val != nullptr) ? val->GetFloat() : deflt;
		memset(buffer, 0, sizeof(buffer));
	}

	return ret;
}

double JSONNode::PullDouble(const char* name, double deflt)
{
	double ret = 0;

	if (name != nullptr)
	{
		char buffer[RAPIDJSON_MAX_PATH_BUFFER];
		int len = sprintf_s(buffer, "%s/%s", pointerPath.c_str(), name);
		rapidjson::Value* val = rapidjson::Pointer(buffer).Get(config->document);
		ret = (val != nullptr) ? val->GetDouble() : deflt;
		memset(buffer, 0, sizeof(buffer));
	}

	return ret;
}

const char*	JSONNode::PullString(const char* name, const char* deflt)
{
	const char* ret = nullptr;

	if (name != nullptr)
	{
		std::string str(pointerPath);
		str += "/";
		str += name;

		//char buffer[RAPIDJSON_MAX_PATH_BUFFER];
		//int len = sprintf_s(buffer, "%s/%s", pointerPath, name);
		rapidjson::Value* val = rapidjson::Pointer(str.c_str()).Get(config->document);
		ret = (val != nullptr) ? val->GetString() : deflt;
		//memset(buffer, 0, sizeof(buffer));
	}

	return ret;
}

//JSONNode PullJObject(const char* name) const;

inline bool JSONNode::operator!() const
{
	return (config == nullptr) || pointerPath.empty();
}

////////////////////////////////////////////////////////////////////
///////////////////FileSystem
////////////////////////////////////////////////////////////////////

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
		engine_config = new Config(SDL_GetBasePath(), "Assets\\config.json");
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
	AddPath(".");
	
	

	+ load config node
	+ if no config create file and save default values*/
}

Config* FileSystem::GetConfig() const
{
	return engine_config;
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
///////////////////FileInfo
////////////////////////////////////////////////////////////////////

FileInfo::FileInfo()
{
	path.clear();
	name.clear();
}

FileInfo::FileInfo(const char* _name, const bool includesPath)
{
	if (includesPath)
	{
		SetFromFullPath(_name);
	}
	else
	{
		path = SDL_GetBasePath();
		name = _name;
	}
}

FileInfo::FileInfo(const char* _path, const char* _name) : name(_name)
{
	path = SDL_GetBasePath();
}

FileInfo::FileInfo(const FileInfo &copy) : path(copy.path), name(copy.name)
{}

bool FileInfo::SetFromFullPath(const char* fullpath)
{
	/* TODO: separate filename from path
	if(fullpath is valid)
	{
		set path and name
	}
	else
	{
		path.clear();
		name.clear();
	}*/

	return !!this;
}

inline bool FileInfo::operator!() const
{
	return path.empty() || name.empty();;
}


////////////////////////////////////////////////////////////////////
///////////////////RE_FileIO
////////////////////////////////////////////////////////////////////

RE_FileIO::RE_FileIO(const char* name, const bool includesPath) : buffer(nullptr)
{
	if (includesPath)
	{
		fileInfo.SetFromFullPath(name);
	}
	else
	{
		fileInfo.path = SDL_GetBasePath();
		fileInfo.name = name;
	}
}

RE_FileIO::RE_FileIO(const char* path, const char* name): buffer(nullptr)
{
	fileInfo.path = path;
	fileInfo.name = name;
}

RE_FileIO::RE_FileIO(FileInfo file) : buffer(nullptr), fileInfo(file)
{}

RE_FileIO::~RE_FileIO()
{
	delete buffer;
}


bool RE_FileIO::Load()
{
	std::string path(fileInfo.path);
	path += fileInfo.name;
	return (!!fileInfo && (HardLoad(path.c_str(), &buffer) > 0));
}

void RE_FileIO::Save()
{
	//TODO: save buffer to file
}

void RE_FileIO::ClearBuffer()
{
	delete buffer;
	buffer = nullptr;
}

const char* RE_FileIO::GetBuffer() const
{
	return buffer;
}

inline bool RE_FileIO::operator!() const
{
	return !fileInfo || buffer == nullptr;
}

unsigned int RE_FileIO::HardLoad(const char* file, char** buffer)
{
	uint ret = 0;

	PHYSFS_File* fs_file = PHYSFS_openRead(file);

	if (fs_file != NULL)
	{
		PHYSFS_sint64 size = PHYSFS_fileLength(fs_file);

		if (size > 0)
		{
			*buffer = new char[(uint)size];
			PHYSFS_sint64 amountRead = PHYSFS_read(fs_file, *buffer, 1, (PHYSFS_sint32)size);

			if (amountRead != size)
			{
				LOG("File System error while reading from file %s: %s\n", file, PHYSFS_getLastError());
				delete (buffer);
			}
			else
			{
				ret = (uint)amountRead;
			}
		}

		if (PHYSFS_close(fs_file) == 0)
		{
			LOG("File System error while closing file %s: %s\n", file, PHYSFS_getLastError());
		}
	}
	else
	{
		LOG("File System error while opening file %s: %s\n", file, PHYSFS_getLastError());
	}

	return ret;
}

////////////////////////////////////////////////////////////////////
///////////////////Config
////////////////////////////////////////////////////////////////////

//Tutorial http://rapidjson.org/md_doc_tutorial.html

Config::Config(const char* name, const bool includesPath) : RE_FileIO(name, includesPath)
{}

Config::Config(const char* path, const char* name) : RE_FileIO(path, name)
{}

Config::Config(FileInfo file) : RE_FileIO(file)
{}

Config::~Config()
{
	//delete document;
}

bool Config::Load()
{
	// Open file
	bool ret = false;
	FILE* fp = nullptr;
	std::string fullPath(fileInfo.path + fileInfo.name);
	if (fopen_s(&fp, fullPath.c_str(), "rb") == 0);// non-Windows use "r"
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
	return !fileInfo || document.IsNull();
}

/*
void Config::Init()
{
	char* path = SDL_GetBasePath();

	std::string test(SDL_GetBasePath());
	test += "Assets\\config.json";


	FILE* fp = nullptr;
	fopen_s(&fp, test.c_str(), "rb");// non-Windows use "r"
	char readBuffer[65536];
	FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	Document d;
	d.ParseStream(is);
	fclose(fp);

	if (d.IsObject())
	{
		if (d.HasMember("engine"))
		{
			Value::ConstMemberIterator itr = d.FindMember("engine");

			if (itr->value.HasMember("name_engine"))
				LOG("The name engine is: %s", itr->value["name_engine"].GetString());
		}
	}

	d.Clear();


	SDL_free(path);
}

bool Config::LoadJsons()
{
	LOG("Loading config.json");
	char* path = SDL_GetBasePath();

	std::string test(SDL_GetBasePath());
	test += "Assets\\config.json";

	FILE* fp = nullptr;
	fopen_s(&fp, test.c_str(), "rb");// non-Windows use "r"
	char readBuffer[65536];
	FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	p.ParseStream(is);
	fclose(fp);

	if (!config->IsObject())
	{
		LOG("Can't load config");
		return false;
	}

	return true;
}

bool Config::LoadConfig()
{
	LOG("Loading config.json");
	char* path = SDL_GetBasePath();

	std::string test(SDL_GetBasePath());
	test += "Assets\\config.json";

	FILE* fp = nullptr;
	fopen_s(&fp, test.c_str(), "rb");// non-Windows use "r"
	char readBuffer[65536];
	FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	config->ParseStream(is);
	fclose(fp);

	return config != nullptr;
}

/*void Config::TestRead()
{
	const char* json = "{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3,4]}";

	Document document;
	document.Parse(json);

		//check root
	assert(document.IsObject());

		//check hello
	assert(document.HasMember("hello"));
	assert(document["hello"].IsString());
	LOG("hello = %s\n", document["hello"].GetString());

		//union test
	//CValue cvalue;
	//cvalue.c = document["hello"].GetString();

		//check bool
	assert(document["t"].IsBool());
	LOG("t = %s\n", document["t"].GetBool() ? "true" : "false");

	//cvalue.b = document["t"].GetBool();
	
		//check null
	LOG("n = %s\n", document["n"].IsNull() ? "null" : "?");

		//check numbers
	assert(document["i"].IsNumber());

	// In this case, IsUint()/IsInt64()/IsUInt64() also return true.
	assert(document["i"].IsInt());
	LOG("i = %d\n", document["i"].GetInt());
	// Alternative (int)document["i"]

	assert(document["pi"].IsNumber());
	assert(document["pi"].IsDouble());
	LOG("pi = %g\n", document["pi"].GetDouble());

		//check array
	// Using a reference for consecutive access is handy and faster.
	const Value& a = document["a"];
	assert(a.IsArray());
	for (SizeType i = 0; i < a.Size(); i++)
	{
		LOG("a[%d] = %d\n", i, a[i].GetInt());
	}

		//Query array
	for (Value::ConstValueIterator itr = a.Begin(); itr != a.End(); ++itr)
		LOG("%d ", itr->GetInt());
	//other functions
	// SizeType Capacity() const;
	// bool Empty() const

	//Range - based For Loop
	for (auto& v : a.GetArray())
		LOG("%d ", v.GetInt());

		//Query Object
	static const char* kTypeNames[] =
	{ "Null", "False", "True", "Object", "Array", "String", "Number" };

	for (Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr)
	{
		LOG("Type of member %s is %s\n",
			itr->name.GetString(), kTypeNames[itr->value.GetType()]);
	}

		//find member
	Value::ConstMemberIterator itr = document.FindMember("hello");
	if (itr != document.MemberEnd()) //check
		LOG("%s\n", itr->value.GetString());

		//Comparing Numbers
	//if (document["hello"] == document["n"]) /*...*/;    // Compare values
	//if (document["hello"] == "world") /*...*/;          // Compare value with literal string
	//if (document["i"] != 123) /*...*/;                  // Compare with integers
	//if (document["pi"] != 3.14) /*...*/;                // Compare with double.
	//Duplicated member name is always false.
/*}

void Config::TestWrite()
{
	{//Change value Type
		Document d; //null
		d.SetObject();

		Value v;
		v.SetInt(10);
		//Same -> v = 10;
	}
	{//Overloaded constructors
		Value b(true);    // calls Value(bool)
		Value i(-123);    // calls Value(int)
		Value u(123u);    // calls Value(unsigned)
		Value d(1.5);     // calls Value(double)

		//empty object or array
		Value o(kObjectType);
		Value a(kArrayType);
	}

		//Move Semantics
	{
		Value a(123);
		Value b(456);
		b = a;         // a becomes a Null value, b becomes number 123.
	}
	{
		Document d;
		Value o(kObjectType);
		{
			Value contacts(kArrayType);
			// adding elements to contacts array.
			o.AddMember("contacts", contacts, d.GetAllocator());  // just memcpy() of contacts itself to the value of new member (16 bytes)
																  // contacts became Null here. Its destruction is trivial.
		}
	}
	{//  Move semantics and temporary values
		Document document;
		Value a(kArrayType);
		Document::AllocatorType& allocator = document.GetAllocator();
		// a.PushBack(Value(42), allocator);       // will not compile
		a.PushBack(Value().SetInt(42), allocator); // fluent API
		a.PushBack(Value(42).Move(), allocator);   // same as above
	}

	//Create String
	{
		Document document;
		Value author;
		char buffer[10];
		int len = sprintf_s(buffer, "%s %s", "Milo", "Yip"); // dynamically created string.
		author.SetString(buffer, len, document.GetAllocator());
		memset(buffer, 0, sizeof(buffer));
		// author.GetString() still contains "Milo Yip" after buffer is destroyed
	}
	{
		Value s;
		s.SetString("rapidjson");    // can contain null character, length derived at compile time
		s = "rapidjson";             // shortcut, same as above
	}
	/*
	{
		const char * cstr = getenv("USER");
		size_t cstr_len = 5;                 // in case length is available
		Value s;
		// s.SetString(cstr);                  // will not compile
		s.SetString(StringRef(cstr));          // ok, assume safe lifetime, null-terminated
		s = StringRef(cstr);                   // shortcut, same as above
		s.SetString(StringRef(cstr, cstr_len)); // faster, can contain null character
		s = StringRef(cstr, cstr_len);          // shortcut, same as above
	}
	*/
/*	{//SWAP
		Value a(123);
		Value b("Hello");
		a.Swap(b);
		assert(a.IsString());
		assert(b.IsInt());
	}

}

//reading/writing JSON rapidjson::Stream
void Config::TestStreams()
{
	Document d;
	const char json[] = "[1, 2, 3, 4]";
		//MemoryStream
	//Input  StringStream
	{ //SStringStream
		StringStream s(json); //GenericStringStream<UTF8<> >
		d.ParseStream(s);
		d.Clear();
	}
	{// cosnt char*
		d.Parse(json);
		d.Clear();
	}
	//Output  StringBuffer 
	{
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		d.Accept(writer);

		const char* output = buffer.GetString();
		/*
		StringBuffer buffer1(0, 1024); // Use its allocator, initial size = 1024
		StringBuffer buffer2(allocator, 1024);

		By default, StringBuffer will instantiate an internal allocator.

		Similarly, StringBuffer is a typedef of GenericStringBuffer<UTF8<> >.
		*/
		/*d.Clear();
	}
	/*
		//File Streams
	{//FileReadStream (Input)
		FILE* fp = fopen("big.json", "rb");// non-Windows use "r"
		char readBuffer[65536];
		FileReadStream is(fp, readBuffer, sizeof(readBuffer));
		d.ParseStream(is);
		fclose(fp);
		d.Clear();
	}
	{// FileWriteStream (Output)
		d.Parse(json);
		FILE* fp = fopen("output.json", "wb"); // non-Windows use "w"
		
		char writeBuffer[65536];
		FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

		Writer<FileWriteStream> writer(os);
		d.Accept(writer);
		fclose(fp);
		d.Clear();
	}
	*/
//}
