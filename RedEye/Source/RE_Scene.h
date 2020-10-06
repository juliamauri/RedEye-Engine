#ifndef __RE_SCENE_H__
#define __RE_SCENE_H__

#include "Resource.h"
class RE_GOManager;

class RE_Scene :
	public ResourceContainer
{
public:
	RE_Scene();
	RE_Scene(const char* metaPath);
	~RE_Scene();

public:
	void LoadInMemory() override;
	void UnloadMemory() override;

	void Import(bool keepInMemory = true) override;

	void Save(RE_GOManager* pool);

	void SetName(const char* name) override;

	//returns a new, needed destroy after use.
	RE_GOManager* GetPool();

private:
	void Draw() override;

	void AssetSave();
	void AssetLoad(bool generateLibraryPath = false);
	void LibraryLoad();
	void LibrarySave(bool fromLoaded = false);

private:
	RE_GOManager* loaded = nullptr;
	RE_GOManager* toSave = nullptr;
};

#endif // !__RE_SCENE_H__