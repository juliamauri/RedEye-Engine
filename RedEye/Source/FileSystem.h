#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "RapidJson\include\document.h"

/* rapidjson number types:

int GetInt()
unsigned int GetUint()
long long GetInt64()
unsigned long long GetUint64()
double GetDouble()
float GetFloat()

bool
string



GenericValue& SetInt(int i)
GenericValue& SetUint(unsigned u)
GenericValue& SetInt64(int64_t i64)
GenericValue& SetUint64(uint64_t u64)
GenericValue& SetDouble(double d)
GenericValue& SetFloat(float f)

*/

#define RAPIDJSON_MAX_PATH_BUFFER 255

class Config;

class JSONNode
{
public:

	JSONNode(const char* path = nullptr, Config* config = nullptr);
	~JSONNode();

	// Push
	bool		PushBool(const char* name, const bool value);
	bool		PushInt(const char* name, const int value);
	bool		PushUInt(const char* name, const unsigned int value);
	bool		PushFloat(const char* name, const float value);
	bool		PushDouble(const char* name, const double value);
	void		PushString(const char* name, const char* value);
	JSONNode	PushJObject(const char* name);

	// Pull
	bool			PullBool(const char* name, bool deflt);
	int				PullInt(const char* name, int deflt);
	unsigned int	PullUInt(const char* name, unsigned int deflt);
	float			PullFloat(const char* name, float deflt);
	double			PullDouble(const char* name, double deflt);
	const char*		PullString(const char* name, const char* deflt);
	JSONNode		PullJObject(const char* name);

	// Utility
	unsigned int Serialize(char** buffer, bool pretty = true);
	inline bool operator!() const;

private:

	Config* config;
	std::string pointerPath;
};

class FileSystem
{
public:

	FileSystem();
	~FileSystem();

	bool Init(int argc, char* argv[]);
	Config* GetConfig() const;

	bool Exists(const char* file) const;
	bool IsDirectory(const char* file) const;
	const char* GetExecutableDirectory() const;

private:

	Config* engine_config;
};

struct FileInfo
{
public:

	FileInfo();
	FileInfo(const char* name, const bool includesPath = false);
	FileInfo(const char* path, const char* name);
	FileInfo(const FileInfo &copy);

	bool SetFromFullPath(const char* fullpath);
	inline bool operator!() const;

	std::string path;
	std::string name;
};

class RE_FileIO
{
public:

	RE_FileIO(const char* name, const bool includesPath = false);
	RE_FileIO(const char* path, const char* name);
	RE_FileIO(FileInfo file);
	~RE_FileIO();

	virtual bool Load();
	virtual void Save();

	void ClearBuffer();
	virtual const char* GetBuffer() const;

	virtual inline bool operator!() const;

private:

	unsigned int HardLoad(const char* file, char** buffer);

protected:

	char* buffer;
	FileInfo fileInfo;
};

class Config : public RE_FileIO
{
public:

	Config(const char* name, const bool includesPath = false);
	Config(const char* path, const char* name);
	Config(FileInfo file);
	~Config();

	bool Load() override;
	JSONNode* GetRootNode(const char* member);
	inline bool operator!() const;

	/*void Init();
	bool LoadJsons();
	bool LoadConfig();

	void TestRead();
	void TestWrite();
	void TestStreams();*/

public:

	rapidjson::Document document;
};

#endif //__FILESYSTEM_H__