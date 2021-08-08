#include "RE_Config.h"

#include "RE_Memory.h"
#include "RE_Assert.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_Json.h"

#include "md5.h"
#include <RapidJson/pointer.h>
#include <RapidJson/stringbuffer.h>
#include <RapidJson/writer.h>

//Tutorial http://rapidjson.org/md_doc_tutorial.html

Config::Config(const char* file_name, const char* _from_zip) : RE_FileBuffer(file_name)
{
	zip_path = _from_zip;
	from_zip = zip_path.c_str();
}

// Open file
bool Config::Load()
{
	bool ret = false;
	size = HardLoad();
	if (ret = (size > 0))
	{
		rapidjson::StringStream s(buffer);
		document.ParseStream(s);
	}
	return ret;
}

bool Config::LoadFromWindowsPath()
{
	// Open file
	bool ret = false;
	RE_FileBuffer* fileToLoad = RE_FS->QuickBufferFromPDPath(file_name);
	if (fileToLoad != nullptr)
	{
		rapidjson::StringStream s(fileToLoad->GetBuffer());
		document.ParseStream(s);
		ret = true;
		DEL(fileToLoad);
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
	eastl::string file(file_name);
	WriteFile(from_zip, file.c_str(), s_buffer.GetString(), size);
}

RE_Json* Config::GetRootNode(const char* member)
{
	RE_ASSERT(member != nullptr);
	eastl::string path("/"); path += member;
	rapidjson::Pointer pointer(path.c_str());
	if (pointer.Get(document) == nullptr) pointer.Create(document);
	return new RE_Json(path.c_str(), this);
}

inline bool Config::operator!() const { return document.IsNull(); }

eastl::string Config::GetMd5()
{
	rapidjson::StringBuffer s_buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s_buffer);
	document.Accept(writer);
	s_buffer.Put('\0');
	size = s_buffer.GetSize();
	return md5(s_buffer.GetString());
}