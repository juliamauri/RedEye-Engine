#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "RapidJson\include\rapidjson.h"
#include "RapidJson\include\document.h"
#include "RapidJson\include\allocators.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4.h"
#include <list>
#include <string>
#include <vector>

class Config;
class RE_FileIO;
class RE_GameObject;
struct Vertex;

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

	const char* GetZipPath();

	void HandleDropedFile(const char* file);

	std::vector<std::string> FindAllFilesByExtension(const char* path, const char* extension, bool repercusive = false);

private:

	std::string RecursiveFindFbx(const char* path);
	std::string RecursiveFindFileOwnFileSystem(const char* directory_path, const char* fileToFind);
	std::string RecursiveFindFileOutsideFileSystem(const char* directory_path, const char* exporting_path, const char* fileToFind);

	//Returns true if same
	bool RecursiveComparePath(const char* path1, const char* path2);

	void RecursiveCopy(const char* origin, const char* dest);

private:

	Config* engine_config;
	std::string engine_path;
	std::string library_path;
	std::string assets_path;
	std::string zip_path;
	std::string write_path;
};

class RE_FileIO
{
public:
	RE_FileIO(const char* file_name, const char* from_zip = nullptr);
	virtual ~RE_FileIO();

	virtual bool Load();
	virtual void Save();
	virtual void Save(char* buffer, unsigned int size = 0);

	void ClearBuffer();
	char* GetBuffer();
	const char* GetBuffer() const;

	virtual inline bool operator!() const;

	virtual unsigned int GetSize();

	virtual std::string GetMd5();

protected:

	unsigned int HardLoad();
	void HardSave(const char* buffer);
	void WriteFile(const char* zip_path, const char* filename, const char * buffer, unsigned int size);

protected:

	const char* from_zip;
	char* buffer = nullptr;
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

	std::string GetMd5() override;

public:
	std::string zip_path;
	rapidjson::Document document;
};

class JSONNode
{
public:

	JSONNode(const char* path = nullptr, Config* config = nullptr, bool isArray = false);
	~JSONNode();

	// Push
	void		PushBool(const char* name, const bool value);
	void		PushInt(const char* name, const int value);
	void		PushUInt(const char* name, const unsigned int value);
	void		PushFloat(const char* name, const float value);
	void		PushFloatVector(const char* name, math::vec vector);
	void		PushFloat4(const char* name, math::float4 vector);
	void		PushDouble(const char* name, const double value);
	void		PushString(const char* name, const char* value);
	void		PushValue(rapidjson::Value* val);
	JSONNode*	PushJObject(const char* name);

	// Pull
	bool			PullBool(const char* name, bool deflt);
	int				PullInt(const char* name, int deflt);
	unsigned int	PullUInt(const char* name, unsigned int deflt);
	float			PullFloat(const char* name, float deflt);
	math::vec		PullFloatVector(const char* name, math::vec deflt);
	math::float4		PullFloat4(const char* name, math::float4 deflt);
	double			PullDouble(const char* name, double deflt);
	const char*		PullString(const char* name, const char* deflt);
	JSONNode*		PullJObject(const char* name);

	//GameObject
	RE_GameObject* FillGO();

	// Utility
	inline bool operator!() const;
	const char* GetDocumentPath() const;
	rapidjson::Document* GetDocument();

	void SetArray();
	void SetObject();

private:

	JSONNode(JSONNode& node);

private:
	Config* config;
	std::string pointerPath;
};

#endif // !__FILESYSTEM_H__