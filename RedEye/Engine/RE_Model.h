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

	void LoadInMemory() override final;
	void UnloadMemory() override final;

	void SetAssetPath(const char* originPath) override final;

	void Import(bool keepInMemory = true) override final;
	RE_ECS_Pool* GetPool();

private:

	void Draw() override;
	void SaveResourceMeta(RE_Json* metaNode) const override final;
	void LoadResourceMeta(RE_Json* metaNode) override final;

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