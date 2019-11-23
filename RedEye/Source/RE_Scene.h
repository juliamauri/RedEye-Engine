#ifndef __RE_SCENE_H__
#define __RE_SCENE_H__

#include "Resource.h"
class RE_GameObject;

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

	void Save(RE_GameObject* go);

	void SetName(const char* name) override;

	//returns a new, needed destroy after use.
	RE_GameObject* GetRoot();

private:
	void Draw() override;

	void AssetSave();
	void AssetLoad();
	void LibraryLoad();
	void LibrarySave();

private:
	RE_GameObject* loaded = nullptr;
	RE_GameObject* toSave = nullptr;
};

#endif // !__RE_SCENE_H__