#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "RapidJson\include\document.h"
#include <list>
#include <string>

class Config;
class RE_FileIO;

class FileSystem
{
public:

	FileSystem();
	~FileSystem();

	bool Init(int argc, char* argv[]);
	Config* GetConfig() const;

	void DrawEditor();
	bool AddPath(const char* path_or_zip, const char* mount_point = nullptr);
	bool RemovePath(const char* path_or_zip);
	bool SetWritePath(const char* dir);
	const char* GetWritePath() const;
	void LogFolderItems(const char* folder);

	RE_FileIO* QuickBufferFromPDPath(const char* full_path); // , char** buffer, unsigned int size);

	bool Exists(const char* file) const;
	bool IsDirectory(const char* file) const;
	const char* GetExecutableDirectory() const;

private:

	Config* engine_config;
	std::string engine_path;
	std::string library_path;
	std::string assets_path;
	std::string write_path;
};

class RE_FileIO
{
public:
	RE_FileIO(const char* file_name, const char* from_zip = nullptr);
	~RE_FileIO();

	virtual bool Load();
	virtual void Save();

	void ClearBuffer();
	char* GetBuffer();
	const char* GetBuffer() const;

	virtual inline bool operator!() const;

	virtual unsigned int GetSize();

protected:

	unsigned int HardLoad();
	void HardSave(const char* buffer);
	void WriteFile(const char* zip_path, const char* filename, const char * buffer, unsigned int size);

protected:

	const char* from_zip;
	char* buffer;
	const char* file_name;
	unsigned int size = 0;
};

class JSONNode;

class Config : public RE_FileIO
{
public:
	Config(const char* file_name, const char* from_zip);

	bool Load() override;
	void Save() override;
	JSONNode* GetRootNode(const char* member);
	inline bool operator!() const override;

public:
	std::string zip_path;
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
	const char* GetDocumentPath() const;

private:

	JSONNode(JSONNode& node);

private:

	Config* config;
	std::string pointerPath;
};

#endif // !__FILESYSTEM_H__