#ifndef __RE_CONFIG_H__
#define __RE_CONFIG_H__

#include "RE_FileBuffer.h"
#include "RapidJson\include\document.h"

class JSONNode;

class Config : public RE_FileBuffer
{
public:
	Config(const char* file_name, const char* from_zip);

	bool Load() override;
	bool LoadFromWindowsPath();
	void Save() override;

	JSONNode* GetRootNode(const char* member);
	inline bool operator!() const override;

	eastl::string GetMd5() override;

public:
	eastl::string zip_path;
	rapidjson::Document document;
};

#endif // !__RE_CONFIG_H__