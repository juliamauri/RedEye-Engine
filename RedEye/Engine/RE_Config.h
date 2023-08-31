#ifndef __RE_CONFIG_H__
#define __RE_CONFIG_H__

#include "RE_FileBuffer.h"
#include <RapidJson/document.h>

class RE_Json;

class Config : public RE_FileBuffer
{
public:
	Config(const char* file_name);
	~Config() final = default;

	bool Load() override final;
	bool LoadFromWindowsPath();
	void Save() override final;

	RE_Json* GetRootNode(const char* member);
	inline bool operator!() const override final;

	eastl::string GetMd5() override final;

public:

	rapidjson::Document document;
};

#endif // !__RE_CONFIG_H__