#ifndef __RE_PREFAB_H__
#define __RE_PREFAB_H__

#include "Resource.h"
class RE_ECS_Pool;

// .refab
class RE_Prefab : public ResourceContainer
{
public:
	RE_Prefab();
	RE_Prefab(const char* metaPath);
	~RE_Prefab();

public:
	void LoadInMemory() override;
	void UnloadMemory() override;

	void Import(bool keepInMemory = true) override;

	void Save(RE_ECS_Pool* pool, bool rootidentity, bool keepInMemory = false);

	//Override from container, when setting name sets the assets path to Assets/Prefabs/name.refab
	//If you want in another path, set directly all path on SetAsetsPath
	void SetName(const char* name) override;

	//returns a new, needed destroy after use.
	RE_ECS_Pool* GetPool();

private:
	void AssetSave();
	void AssetLoad(bool generateLibraryPath = false);
	void LibraryLoad();
	void LibrarySave();

	void Draw() override;


private:
	RE_ECS_Pool* loaded = nullptr;
	RE_ECS_Pool* toSave = nullptr;
};

#endif // !__RE_PREFAB_H__