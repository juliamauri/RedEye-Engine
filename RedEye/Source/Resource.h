#ifndef __RESOURCE_H__
#define __RESOURCE_H__

// disable warning for unsafe copy
#pragma warning(disable : 4996)

class RE_Mesh;
class RE_Shader;
class RE_Texture;
class RE_Primitive;
class JSONNode;

#include <string>

enum Resource_Type : short unsigned int
{
	R_UNDEFINED = 0x00,
	R_SHADER,
	R_PRIMITIVE,
	R_TEXTURE,
	R_MESH,
	R_PREFAB,
	R_SKYBOX,
	R_INTERNALPREFAB,
	R_MATERIAL
};

class ResourceContainer
{
public:
	ResourceContainer();
	ResourceContainer(const char* metaPath);
	virtual ~ResourceContainer();
	const char* GetName() const;
	const char* GetLibraryPath() const;
	const char* GetAssetPath() const;
	const char* GetMetaPath() const;
	const char* GetMD5() const;
	Resource_Type GetType() const;
	
	void SetType(Resource_Type type);
	void SetMD5(const char* md5);
	void SetLibraryPath(const char* path);
	void SetAssetPath(const char* originPath);
	void SetName(const char* name);

	bool isInternal()const { return isinternal; }

	bool isInMemory()const { return inMemory; }
	virtual void LoadInMemory() = 0;
	virtual void UnloadMemory() = 0;

	void SaveMeta();
	void LoadMeta();

	void DrawPropieties();

private:
	virtual void Draw() {}
	virtual void SaveResourceMeta(JSONNode* metaNode) {}
	virtual void LoadResourceMeta(JSONNode* metaNode) {}

private:
	std::string name;
	std::string propietiesName;
	std::string assetPath;
	std::string libraryPath;
	std::string metaPath;
	const char* md5 = nullptr;
	Resource_Type type;


	bool isinternal = false;

protected:
	bool inMemory = false;
};

#endif // !__RESOURCE_H__