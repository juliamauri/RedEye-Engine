#ifndef __RE_MODEL_H__
#define __RE_MODEL_H__

#include "Resource.h"
#include "RE_ModelSettings.h"

class RE_ECS_Pool;

class RE_Model : public ResourceContainer
{
public:
	RE_Model() {}
	RE_Model(const char* metaPath) : ResourceContainer(metaPath) {}
	~RE_Model() {}

	void LoadInMemory() override;
	void UnloadMemory() override;

	void SetAssetPath(const char* originPath)override;

	void Import(bool keepInMemory = true) override;
	RE_ECS_Pool* GetPool();

private:

	void Draw() override;
	void SaveResourceMeta(RE_Json* metaNode) override;
	void LoadResourceMeta(RE_Json* metaNode) override;

	void AssetLoad();
	void LibraryLoad();
	void LibrarySave();

	bool CheckResourcesIsOnAssets();

private:

	RE_ECS_Pool* loaded = nullptr;
	RE_ModelSettings modelSettings,  restoreSettings;
	bool applySave = false,  needReImport = false;
};

#endif // !__RE_MODEL_H__