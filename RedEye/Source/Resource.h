#ifndef __RESOURCE_H__
#define __RESOURCE_H__

// disable warning for unsafe copy
#pragma warning(disable : 4996)

class RE_Json;

#include <EASTL/string.h>

enum Resource_Type : short unsigned int
{
	R_UNDEFINED = 0x00,
	R_SHADER,
	R_TEXTURE,
	R_MESH,
	R_PREFAB,
	R_SKYBOX,
	R_MATERIAL,
	R_MODEL,
	R_SCENE,
	R_PARTICLE_EMITTER,
	R_PARTICLE_EMISSION,
	R_PARTICLE_RENDER,

	MAX_R_TYPES
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
	signed long long GetLastTimeModified()const;

	void SetType(Resource_Type type);
	void SetMD5(const char* md5);
	virtual void SetLibraryPath(const char* path);
	virtual void SetAssetPath(const char* originPath);
	void SetMetaPath(const char* originPath);
	virtual void SetName(const char* name);
	void SetInternal(bool is_internal);

	bool isInternal()const { return isinternal; }

	bool isInMemory()const { return inMemory; }
	virtual void LoadInMemory(){}
	virtual void UnloadMemory(){}

	virtual void Import(bool keepInMemory = true) { }
	virtual void ReImport() { }
	virtual void SomeResourceChanged(const char* resMD5) { }

	void SaveMeta();
	void LoadMeta();

	void DrawPropieties();

private:

	virtual void Draw() {}
	virtual void SaveResourceMeta(RE_Json* metaNode) {}
	virtual void LoadResourceMeta(RE_Json* metaNode) {}

private:

	eastl::string name;
	eastl::string propietiesName;
	eastl::string assetPath;
	eastl::string libraryPath;
	eastl::string metaPath;

	char* md5 = nullptr;
	Resource_Type type = R_UNDEFINED;
	signed long long lastModified = 0;

	bool isinternal = false;

protected:

	bool inMemory = false;
};

#endif // !__RESOURCE_H__