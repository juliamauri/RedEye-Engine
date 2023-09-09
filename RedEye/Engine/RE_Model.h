#ifndef __RE_MODEL_H__
#define __RE_MODEL_H__

#include "RE_ModelSettings.h"

class RE_ECS_Pool;

class RE_Model : public ResourceContainer
{
public:
	RE_Model() = default;
	RE_Model(const char* metaPath) : ResourceContainer(metaPath) {}
	~RE_Model() final = default;

	void LoadInMemory() final;
	void UnloadMemory() final;

	void SetAssetPath(const char* originPath) final;

	void Import(bool keepInMemory = true) final;
	RE_ECS_Pool* GetPool();

private:

	void Draw() override;
	void SaveResourceMeta(RE_Json* metaNode) const final;
	void LoadResourceMeta(RE_Json* metaNode) final;

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