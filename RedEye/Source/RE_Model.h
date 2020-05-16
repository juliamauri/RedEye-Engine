#ifndef __RE_MODEL_H__
#define __RE_MODEL_H__

#include "Resource.h"

#include <EASTL/vector.h>

class RE_GameObject;
enum aiPostProcessSteps;

struct RE_ModelSettings {
	//presets
	bool presets[3] = { false, false, false };
	//bool Preset_TargetRealtime_Fast
	//bool Preset_TargetRealtime_MaxQuality
	//bool Preset_TargetRealtime_Quality

	bool flags[25] = { true, true, true, false, true, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false };
	//0 CalcTangentSpace
	//1 JoinIdenticalVertices
	//2 Triangulate
	//3 RemoveComponent
	//4 GenNormals
	//5 GenSmoothNormals
	// SplitLargeMeshes
	// PreTransformVertices
	// LimitBoneWeights
	// ValidateDataStructure
	//10 ImproveCacheLocality
	// RemoveRedundantMaterials
	// FixInfacingNormals
	// SortByPType
	// FindDegenerates
	//15 FindInvalidData
	// GenUVCoords
	// TransformUVCoords
	// FindInstances
	// OptimizeMeshes
	//20 OptimizeGraph
	// FlipUVs
	// FlipWindingOrder
	// SplitByBoneCount
	//24 Debone
	
	int GetPresetSelected()const;
	unsigned int GetFlags()const;
	eastl::vector<const char*> libraryMeshes;

	inline bool operator==(const RE_ModelSettings& b){
		bool ret = true;
		for (unsigned int i = 0; i < 3; i++) {
			if (presets[i] != b.presets[i]) {
				ret = false;
				break;
			}
		}
		if (ret) {
			for (unsigned int i = 0; i < 25; i++) {
				if (flags[i] != b.flags[i]) {
					ret = false;
					break;
				}
			}
		}
		return ret;
	}
	inline bool operator!=(const RE_ModelSettings& b){
		bool ret = true;
		for (unsigned int i = 0; i < 3; i++) {
			if (presets[i] == b.presets[i]) {
				ret = false;
				break;
			}
		}
		if (ret) {
			for (unsigned int i = 0; i < 25; i++) {
				if (flags[i] == b.flags[i]) {
					ret = false;
					break;
				}
			}
		}
		return ret;
	}
};

class RE_Model :
public ResourceContainer
{
public:
	RE_Model();
	RE_Model(const char* metaPath);
	~RE_Model();

	void LoadInMemory() override;
	void UnloadMemory() override;

	void SetAssetPath(const char* originPath)override;

	void Import(bool keepInMemory = true) override;
	RE_GameObject* GetRoot();

private:
	void Draw() override;
	void SaveResourceMeta(JSONNode* metaNode) override;
	void LoadResourceMeta(JSONNode* metaNode) override;

	void AssetLoad();
	void LibraryLoad();
	void LibrarySave();

	bool CheckResourcesIsOnAssets();

private:
	RE_GameObject* loaded = nullptr;

	RE_ModelSettings modelSettings;

	bool applySave = false;
	RE_ModelSettings restoreSettings;

	bool needReImport = false;
};

#endif // !__RE_MODEL_H__