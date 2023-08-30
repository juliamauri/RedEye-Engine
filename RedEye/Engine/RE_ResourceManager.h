#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include "EventListener.h"
#include "Resource.h"

#include <EASTL/map.h>
#include <EASTL/vector.h>
#include <EASTL/stack.h> 

class RE_ResourceManager : public EventListener
{
public:
	RE_ResourceManager() = default;
	~RE_ResourceManager() final = default;

	void Init();
	void Clear();

	void RecieveEvent(const Event& e) override;

	ResourceContainer* At(const char* md5) const;
	const char* ReferenceByMeta(const char* path, ResourceType type);
	const char* Reference(ResourceContainer* rc);
	size_t TotalReferences() const;

	eastl::vector<const char*> GetAllResourcesActiveByType(ResourceType resT);

	eastl::vector<const char*> WhereUndefinedFileIsUsed(const char* assetPath);
	eastl::vector<const char*> WhereIsUsed(const char* res);
	ResourceContainer* DeleteResource(const char* res, eastl::vector<const char*> resourcesWillChange, bool resourceOnScene);

	const char* ImportModel(const char* assetPath);
	const char* ImportTexture(const char* assetPath);
	const char* ImportMaterial(const char* assetPath);
	const char* ImportSkyBox(const char* assetPath);
	const char* ImportPrefab(const char* assetPath);
	const char* ImportScene(const char* assetPath);
	//TODO IMPORT PARTICLE RESOURCES
	const char* ImportParticleEmissor(const char* assetPath);
	const char* ImportParticleRender(const char* assetPath);

	unsigned int TotalReferenceCount(const char* resMD5) const;
	void Use(const char* resMD5);
	void UnUse(const char* resMD5);

	void PushSelected(const char* resS, bool popAll = false);
	const char* GetSelected()const;
	void PopSelected(bool all = false);

	eastl::vector<ResourceContainer*> GetResourcesByType(ResourceType type) const;
	const char* IsReference(const char* md5, ResourceType type = ResourceType::UNDEFINED);
	const char* FindMD5ByMETAPath(const char* metaPath, ResourceType type = ResourceType::UNDEFINED);
	const char* FindMD5ByLibraryPath(const char* libraryPath, ResourceType type = ResourceType::UNDEFINED);
	const char* FindMD5ByAssetsPath(const char* assetsPath, ResourceType type = ResourceType::UNDEFINED);
	const char* CheckOrFindMeshOnLibrary(const char* librariPath);

	bool isNeededResoursesLoaded(const char* metaPath, ResourceType type)const;

	void ThumbnailResources();

	void PushParticleResource(const char* md5);
	void ProcessParticlesReimport();

	typedef eastl::pair<const char*, ResourceContainer*> Resource;
	typedef eastl::map<const char*, ResourceContainer*> ResourceMap;
	typedef ResourceMap::iterator ResourceIter;
	typedef ResourceMap::const_iterator ResourceConstIter;
	
	typedef eastl::pair<const char*, unsigned int> ResourceCounter;
	typedef eastl::map<const char*, unsigned int> ResourceCounterMap;

private:

	const char* GetNameFromType(const ResourceType type);

private:

	ResourceMap resources;
	ResourceCounterMap resourcesCounter;

	eastl::stack< const char*> resourcesSelected;
	eastl::stack< const char*> resources_particles_reimport;
};

#endif // !__RESOURCEMANAGER_H__