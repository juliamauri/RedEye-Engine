#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "RapidJson\include\rapidjson.h"
#include "RapidJson\include\document.h"
#include "RapidJson\include\allocators.h"

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/float3x3.h"
#include "MathGeoLib/include/Math/float4x4.h"
#include <EASTL/list.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/stack.h>

class Config;
class RE_FileIO;
class RE_GameObject;
class ResourceContainer;
struct Vertex;

class RE_FileSystem
{
public:

	enum PathType {
		D_NULL = -1,
		D_FOLDER,
		D_FILE
	};

	enum FileType {
		F_NOTSUPPORTED = -1,
		F_NONE,
		F_MODEL,
		F_TEXTURE,
		F_MATERIAL,
		F_SKYBOX,
		F_PREFAB,
		F_SCENE,
		F_META
	};

	enum PathProcessType {
		P_ADDFILE,
		P_DELETE,
		P_REIMPORT,
		P_ADDFOLDER,
	};

	struct RE_File;
	struct RE_Meta;
	struct RE_Directory;

	struct RE_Path {
		eastl::string path;
		PathType pType;

		RE_File* AsFile()const { return (RE_File*)this; }
		RE_Meta* AsMeta()const { return (RE_Meta*)this; }
		RE_Directory* AsDirectory()const { return (RE_Directory*)this; }
	};

	struct RE_File : public RE_Path
	{
		eastl::string filename;
		FileType fType = F_NONE;
		const char* extension = nullptr;
		signed long long lastModified = 0;
		signed long long lastSize = 0;

		RE_Meta* metaResource = nullptr;

		static FileType DetectExtensionAndType(const char* _path, const char*& _extension);

		RE_Path* AsPath()const { return (RE_Path*)this; }
		RE_Meta* AsMeta()const { return (RE_Meta*)this; }
	};

	struct RE_Meta :public RE_File
	{
		RE_File* fromFile = nullptr;
		const char* resource = nullptr;
		
		bool IsModified()const;

		RE_Path* AsPath()const { return (RE_Path*)this; }
		RE_File* AsFile()const { return (RE_File*)this; }
	};

	struct RE_ProcessPath {
		PathProcessType whatToDo;
		RE_Path* toProcess;
	};

	struct RE_Directory : public RE_Path
	{
		eastl::string name;
		typedef eastl::list<RE_Path*>::iterator pathIterator;
		RE_Directory* parent = nullptr;
		eastl::list<RE_Path*> tree;

		void AddBeforeOf(RE_Path* toAdd, pathIterator to) { tree.insert(to, toAdd); }
		void Delete(eastl::list<RE_Path*>::iterator del) { tree.erase(del); }

		void SetPath(const char* path);
		eastl::list< RE_Directory*> MountTreeFolders();
		eastl::stack<RE_ProcessPath*> CheckAndApply(eastl::vector<RE_Meta*>* metaRecentlyAdded);

		eastl::stack<RE_Path*> GetDisplayingFiles()const;

		eastl::list<RE_Directory*> FromParentToThis();

		RE_Path* AsPath()const { return (RE_Path*)this; }
	};

public:

	RE_FileSystem();
	~RE_FileSystem();

	bool Init(int argc, char* argv[]);
	Config* GetConfig() const;

	unsigned int ReadAssetChanges(unsigned int extra_ms, bool doAll = false);

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

	RE_Directory* GetRootDirectory()const;

private:
	void RecursiveCopy(const char* origin, const char* dest);

private:

	Config* engine_config;
	eastl::string engine_path;
	eastl::string library_path;
	eastl::string assets_path;
	eastl::string zip_path;
	eastl::string write_path;

	RE_Directory* rootAssetDirectory = nullptr;
	eastl::list< RE_Directory*> assetsDirectories;
	eastl::list< RE_Directory*>::iterator dirIter;
	eastl::stack<RE_ProcessPath*> assetsToProcess;

	eastl::vector<RE_Meta*> metaToFindFile;
	eastl::vector<RE_File*> filesToFindMeta;

	eastl::vector<RE_Meta*> metaRecentlyAdded;

	eastl::list<RE_File*> toImport;
	eastl::list<RE_Meta*> toReImport;
};

class RE_FileIO
{
public:
	RE_FileIO(const char* file_name, const char* from_zip = nullptr);
	virtual ~RE_FileIO();

	virtual bool Load();
	virtual void Save();
	virtual void Save(char* buffer, unsigned int size = 0);

	void Delete();

	void ClearBuffer();
	char* GetBuffer();
	const char* GetBuffer() const;

	virtual inline bool operator!() const;

	virtual unsigned int GetSize();

	virtual eastl::string GetMd5();

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

	eastl::string GetMd5() override;

public:
	eastl::string zip_path;
	rapidjson::Document document;
};

class JSONNode
{
public:

	JSONNode(const char* path = nullptr, Config* config = nullptr, bool isArray = false);
	~JSONNode();

	// Push
	void		PushBool(const char* name, const bool value);
	void		PushBool(const char* name, const bool* value, unsigned int quantity);
	void		PushInt(const char* name, const int value);
	void		PushInt(const char* name, const int* value, unsigned int quantity);
	void		PushUInt(const char* name, const unsigned int value);
	void		PushUInt(const char* name, const unsigned int* value, unsigned int quantity);
	void		PushFloat(const char* name, const float value);
	void		PushFloat(const char* name, math::float2 value);
	void		PushFloatVector(const char* name, math::vec vector);
	void		PushFloat4(const char* name, math::float4 vector);
	void		PushMat3(const char* name, math::float3x3 mat3);
	void		PushMat4(const char* name, math::float4x4 mat4);
	void		PushDouble(const char* name, const double value);
	void		PushSignedLongLong(const char* name, const signed long long value);
	void		PushString(const char* name, const char* value);
	void		PushValue(rapidjson::Value* val);
	JSONNode*	PushJObject(const char* name);

	// Pull
	bool			PullBool(const char* name, bool deflt);
	bool*			PullBool(const char* name, unsigned int quantity, bool deflt);
	int				PullInt(const char* name, int deflt);
	int*			PullInt(const char* name, unsigned int quantity, int deflt);
	unsigned int	PullUInt(const char* name, unsigned int deflt);
	unsigned int*	PullUInt(const char* name, unsigned int quantity, unsigned int deflt);
	float			PullFloat(const char* name, float deflt);
	math::float2	PullFloat(const char* name, math::float2 deflt);
	math::vec		PullFloatVector(const char* name, math::vec deflt);
	math::float4	PullFloat4(const char* name, math::float4 deflt);
	math::float3x3	PullMat3(const char* name, math::float3x3 deflt);
	math::float4x4	PullMat4(const char* name, math::float4x4 deflt);
	double			PullDouble(const char* name, double deflt);
	signed long long	PullSignedLongLong(const char* name, signed long long deflt);

	const char*		PullString(const char* name, const char* deflt);
	JSONNode*		PullJObject(const char* name);

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
	eastl::string pointerPath;
};

#endif // !__FILESYSTEM_H__