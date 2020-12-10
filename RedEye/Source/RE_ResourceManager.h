#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include "Resource.h"
#include "RE_InternalResources.h"

#include <EASTL/map.h>
#include <EASTL/vector.h>
#include <EASTL/stack.h> 

namespace RE_ResourceManager
{
	void Init();
	void Clear();
	void ResourceHasChanged(const char* md5);

	ResourceContainer* At(const char* md5);
	const char* ReferenceByMeta(const char* path, Resource_Type type);
	const char* Reference(ResourceContainer* rc);
	unsigned int TotalReferences();

	eastl::vector<const char*> GetAllResourcesActiveByType(Resource_Type resT);

	eastl::vector<const char*> WhereUndefinedFileIsUsed(const char* assetPath);
	eastl::vector<const char*> WhereIsUsed(const char* res);
	ResourceContainer* DeleteResource(const char* res, eastl::vector<const char*> resourcesWillChange, bool resourceOnScene);

	const char* ImportModel(const char* assetPath);
	const char* ImportTexture(const char* assetPath);
	const char* ImportMaterial(const char* assetPath);
	const char* ImportSkyBox(const char* assetPath);
	const char* ImportPrefab(const char* assetPath);
	const char* ImportScene(const char* assetPath);

	unsigned int TotalReferenceCount(const char* resMD5);
	void Use(const char* resMD5);
	void UnUse(const char* resMD5);

	void PushSelected(const char* resS, bool popAll = false);
	const char* GetSelected();
	void PopSelected(bool all = false);

	eastl::vector<ResourceContainer*> GetResourcesByType(Resource_Type type);
	const char* IsReference(const char* md5, Resource_Type type = Resource_Type::R_UNDEFINED);
	const char* FindMD5ByMETAPath(const char* metaPath, Resource_Type type = Resource_Type::R_UNDEFINED);
	const char* FindMD5ByLibraryPath(const char* libraryPath, Resource_Type type = Resource_Type::R_UNDEFINED);
	const char* FindMD5ByAssetsPath(const char* assetsPath, Resource_Type type = Resource_Type::R_UNDEFINED);
	const char* CheckOrFindMeshOnLibrary(const char* librariPath);

	void ThumbnailResources();

	typedef eastl::pair<const char*, ResourceContainer*> Resource;
	typedef eastl::map<const char*, ResourceContainer*> ResourceMap;
	typedef ResourceMap::iterator ResourceIter;
	typedef ResourceMap::const_iterator ResourceConstIter;

	typedef eastl::pair<const char*, unsigned int> ResourceCounter;
	typedef eastl::map<const char*, unsigned int> ResourceCounterMap;

	namespace Internal
	{
		const char* GetNameFromType(const Resource_Type type);

		static ResourceMap resources;
		static ResourceCounterMap resourcesCounter;
		static eastl::stack< const char*> resourcesSelected;
	}
}

#endif // !__RESOURCEMANAGER_H__