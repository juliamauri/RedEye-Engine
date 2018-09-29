#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "RapidJson\include\document.h"

class Config;
class RE_FileIO;

class FileSystem
{
public:

	FileSystem();
	~FileSystem();

	bool Init(int argc, char* argv[]);
	Config* GetConfig() const;

	bool AddPath(const char* path_or_zip, const char* mount_point = nullptr);
	bool Exists(const char* file) const;
	bool IsDirectory(const char* file) const;
	const char* GetExecutableDirectory() const;

private:

	Config* engine_config;
};


/* Access file buffer:
RE_FileIO file("config.json");
	if (file.Load())
		const char* buffer = file.GetBuffer();*/

class RE_FileIO
{
public:
	RE_FileIO(const char* file_name);
	~RE_FileIO();

	virtual bool Load();
	virtual void Save();

	void ClearBuffer();
	virtual const char* GetBuffer() const;

	virtual inline bool operator!() const;

	virtual unsigned int GetSize();

protected:

	unsigned int HardLoad();
	void HardSave(const char* buffer);

protected:

	char* buffer;
	const char* file_name;
	unsigned int size = 0;
};

class JSONNode;

class Config : public RE_FileIO
{
public:
	Config(const char* file_name);

	bool Load() override;
	void Save() override;
	JSONNode* GetRootNode(const char* member);
	inline bool operator!() const override;

public:

	rapidjson::Document document;
};

class JSONNode
{
public:

	JSONNode(const char* path = nullptr, Config* config = nullptr);
	~JSONNode();

	// Push
	void		PushBool(const char* name, const bool value);
	void		PushInt(const char* name, const int value);
	void		PushUInt(const char* name, const unsigned int value);
	void		PushFloat(const char* name, const float value);
	void		PushDouble(const char* name, const double value);
	void		PushString(const char* name, const char* value);
	JSONNode*	PushJObject(const char* name);

	// Pull
	bool			PullBool(const char* name, bool deflt);
	int				PullInt(const char* name, int deflt);
	unsigned int	PullUInt(const char* name, unsigned int deflt);
	float			PullFloat(const char* name, float deflt);
	double			PullDouble(const char* name, double deflt);
	const char*		PullString(const char* name, const char* deflt);
	JSONNode*		PullJObject(const char* name);

	// Utility
	inline bool operator!() const;

private:

	JSONNode(JSONNode& node);

private:

	Config* config;
	std::string pointerPath;
};

#endif // !__FILESYSTEM_H__