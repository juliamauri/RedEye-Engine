#ifndef __RE_SCENE_H__
#define __RE_SCENE_H__

class RE_ECS_Pool;

class RE_Scene : public ResourceContainer
{
public:
	RE_Scene() = default;
	RE_Scene(const char* metaPath) : ResourceContainer(metaPath) {}
	~RE_Scene() final = default;

public:
	void LoadInMemory() final;
	void UnloadMemory() final;

	void Import(bool keepInMemory = true) final;

	void Save(RE_ECS_Pool* pool);

	void SetName(const char* name) final;

	//returns a new, needed destroy after use.
	RE_ECS_Pool* GetPool();

private:
	void Draw() final;

	void AssetSave();
	void AssetLoad(bool generateLibraryPath = false);
	void LibraryLoad();
	void LibrarySave(bool fromLoaded = false);

private:
	RE_ECS_Pool* loaded = nullptr;
	RE_ECS_Pool* toSave = nullptr;
};

#endif // !__RE_SCENE_H__