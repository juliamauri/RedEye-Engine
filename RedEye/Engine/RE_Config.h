#ifndef __RE_CONFIG_H__
#define __RE_CONFIG_H__

#include "RE_FileBuffer.h"
#include <RapidJson/document.h>

class RE_Json;

class Config : public RE_FileBuffer
{
public:
	Config(const char* file_name);

	bool Load() override;
	bool LoadFromWindowsPath();
	void Save() override;

	RE_Json* GetRootNode(const char* member);
	inline bool operator!() const override;

	eastl::string GetMd5() override;

public:

	rapidjson::Document document;
};

#endif // !__RE_CONFIG_H__