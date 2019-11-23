#ifndef __RE_MODEL_H__
#define __RE_MODEL_H__

#include "Resource.h"

#include <vector>

class RE_GameObject;
enum aiPostProcessSteps;

struct RE_ModelSettings {
	//presets
	bool presets[3] = { false, false, true };
	//bool Preset_TargetRealtime_Fast
	//bool Preset_TargetRealtime_MaxQuality
	//bool Preset_TargetRealtime_Quality

	bool flags[25] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
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
	// GenUVCoords
	// TransformUVCoords
	// FindInstances
	//20 OptimizeMeshes
	// OptimizeGraph
	// FlipUVs
	// FlipWindingOrder
	// SplitByBoneCount
	//25 Debone

	unsigned int GetFlags()const;
	std::vector<const char*> libraryMeshes;
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

	void Import(bool keepInMemory = true);
	RE_GameObject* GetRoot();

private:
	void Draw() override;
	void SaveResourceMeta(JSONNode* metaNode) override;
	void LoadResourceMeta(JSONNode* metaNode) override;

	void AssetLoad();
	void LibraryLoad();
	void LibrarySave();

private:
	RE_GameObject* loaded = nullptr;

	RE_ModelSettings modelSettings;

	bool applySave = false;
	RE_ModelSettings restoreSettings;
};

#endif // !__RE_MODEL_H__