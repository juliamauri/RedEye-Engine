#ifndef __RESOURCE_H__
#define __RESOURCE_H__

// disable warning for unsafe copy
#pragma warning(disable : 4996)

class RE_Json;

#include "RE_DataTypes.h"
#include <EASTL/string.h>

class ResourceContainer
{
public:

	enum class Type : ushort
	{
		UNDEFINED = 0x00,

		SHADER,
		TEXTURE,
		MESH,
		PREFAB,
		SKYBOX,
		MATERIAL,
		MODEL,
		SCENE,
		PARTICLE_EMITTER,
		PARTICLE_EMISSION,
		PARTICLE_RENDER,

		MAX
	};

	ResourceContainer();
	ResourceContainer(const char* metaPath);
	virtual ~ResourceContainer();
	const char* GetName() const;
	const char* GetLibraryPath() const;
	const char* GetAssetPath() const;
	const char* GetMetaPath() const;
	const char* GetMD5() const;
	const Type GetType() const;
	const signed long long GetLastTimeModified() const;

	void SetType(const Type type);
	void SetMD5(const char* md5);
	virtual void SetLibraryPath(const char* path);
	virtual void SetAssetPath(const char* originPath);
	void SetMetaPath(const char* originPath);
	virtual void SetName(const char* name);
	void SetInternal(bool is_internal);

	bool isInternal() const { return isinternal; }
	bool isInMemory() const { return inMemory; }
	virtual void LoadInMemory(){}
	virtual void UnloadMemory(){}

	virtual void Import(bool keepInMemory = true) { }
	virtual void ReImport() { }
	virtual void SomeResourceChanged(const char* resMD5) { }

	void SaveMeta();
	void LoadMeta();

	bool CheckResourcesReferenced();

	void DrawPropieties();

	template <typename T> T As() const { return dynamic_cast<T>(this); }

private:

	virtual void Draw() {}
	virtual void SaveResourceMeta(RE_Json* metaNode) const {}
	virtual void LoadResourceMeta(RE_Json* metaNode) {}

	virtual bool NeededResourcesReferenced(RE_Json* metaNode) { return false; }

private:

	eastl::string name;
	eastl::string propietiesName;
	eastl::string assetPath;
	eastl::string libraryPath;
	eastl::string metaPath;

	char* md5 = nullptr;
	Type type = Type::UNDEFINED;
	signed long long lastModified = 0;

	bool isinternal = false;

protected:

	bool inMemory = false;
};

#endif // !__RESOURCE_H__